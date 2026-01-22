/*
 * dcc_signal.c
 *
 *  Created on: Jan 22, 2026
 *      Author: J. Früh
 */

#include <dcc_signal.h>

#include <dcc_protocol_rx.h>
#include <m_memory.h>


DCC_SIGNAL_STRUCT DccSignal;

extern DCC_INSTRUCTION_STRUCT DccInst;
extern struct MEM_CONFIG_STRUCT Mem;



static const int16_t signal_speed_limit[] =
{
	[SIGNAL_STOP] = 0,
	[SIGNAL_40KMH] = 5,
	[SIGNAL_60KMH] = 8,
	[SIGNAL_90KMH] = 12,
	[SIGNAL_FREE] = 28
};


void signal_speed_limit_update()
{
	// Update speed limit
	if(DccInst.signal_state != SIGNAL_NOCHANGE)
		DccInst.speed_limit = signal_speed_limit[DccInst.signal_state];
	else if (DccInst.speed_limit < signal_speed_limit[SIGNAL_40KMH])
		DccInst.speed_limit = signal_speed_limit[SIGNAL_40KMH];
}


void signal_update()
{

	while(DccSignal.out_idx != DccSignal.in_idx)
	{
		uint32_t timeout = DccSignal.timeout_tab[DccSignal.out_idx];

		if(timeout < TIMEOUT__THRES_FREE)
		{
			if(DccSignal.free_cnt < 10)
				DccSignal.free_cnt++;

			if(DccSignal.nochange_cnt > 0)
				DccSignal.nochange_cnt--;
			if(DccSignal.sp90kmh_cnt > 0)
				DccSignal.sp90kmh_cnt--;
			if(DccSignal.sp60kmh_cnt > 0)
				DccSignal.sp60kmh_cnt--;
			if(DccSignal.sp40kmh_cnt > 0)
				DccSignal.sp40kmh_cnt--;
			if(DccSignal.stop_cnt > 0)
				DccSignal.stop_cnt--;
		}
		else if(timeout < TIMEOUT__THRES_NOCHANGE)
		{
			if(DccSignal.nochange_cnt < 10)
				DccSignal.nochange_cnt++;

			if(DccSignal.free_cnt > 0)
				DccSignal.free_cnt--;
			if(DccSignal.sp90kmh_cnt > 0)
				DccSignal.sp90kmh_cnt--;
			if(DccSignal.sp60kmh_cnt > 0)
				DccSignal.sp60kmh_cnt--;
			if(DccSignal.sp40kmh_cnt > 0)
				DccSignal.sp40kmh_cnt--;
			if(DccSignal.stop_cnt > 0)
				DccSignal.stop_cnt--;
		}
		else if(timeout < TIMEOUT__THRES_90KMH)
		{
			if(DccSignal.sp90kmh_cnt < 10)
				DccSignal.sp90kmh_cnt++;

			if(DccSignal.free_cnt > 0)
				DccSignal.free_cnt--;
			if(DccSignal.nochange_cnt > 0)
				DccSignal.nochange_cnt--;
			if(DccSignal.sp60kmh_cnt > 0)
				DccSignal.sp60kmh_cnt--;
			if(DccSignal.sp40kmh_cnt > 0)
				DccSignal.sp40kmh_cnt--;
			if(DccSignal.stop_cnt > 0)
				DccSignal.stop_cnt--;
		}
		else if(timeout < TIMEOUT__THRES_60KMH)
		{
			if(DccSignal.sp60kmh_cnt < 10)
				DccSignal.sp60kmh_cnt++;

			if(DccSignal.free_cnt > 0)
				DccSignal.free_cnt--;
			if(DccSignal.nochange_cnt > 0)
				DccSignal.nochange_cnt--;
			if(DccSignal.sp90kmh_cnt > 0)
				DccSignal.sp90kmh_cnt--;
			if(DccSignal.sp40kmh_cnt > 0)
				DccSignal.sp40kmh_cnt--;
			if(DccSignal.stop_cnt > 0)
				DccSignal.stop_cnt--;
		}
		else if(timeout < TIMEOUT__THRES_40KMH)
		{
			if(DccSignal.sp40kmh_cnt < 10)
				DccSignal.sp40kmh_cnt++;

			if(DccSignal.free_cnt > 0)
				DccSignal.free_cnt--;
			if(DccSignal.nochange_cnt > 0)
				DccSignal.nochange_cnt--;
			if(DccSignal.sp90kmh_cnt > 0)
				DccSignal.sp90kmh_cnt--;
			if(DccSignal.sp60kmh_cnt > 0)
				DccSignal.sp60kmh_cnt--;
			if(DccSignal.stop_cnt > 0)
				DccSignal.stop_cnt--;
		}
		else if(timeout < TIMEOUT__THRES_STOP)
		{
			if(DccSignal.stop_cnt < 10)
				DccSignal.stop_cnt++;

			if(DccSignal.free_cnt > 0)
				DccSignal.free_cnt--;
			if(DccSignal.nochange_cnt > 0)
				DccSignal.nochange_cnt--;
			if(DccSignal.sp90kmh_cnt > 0)
				DccSignal.sp90kmh_cnt--;
			if(DccSignal.sp60kmh_cnt > 0)
				DccSignal.sp60kmh_cnt--;
			if(DccSignal.sp40kmh_cnt > 0)
				DccSignal.sp40kmh_cnt--;
		}

		signal_state_t signal_state_new = DccInst.signal_state;

		if(DccInst.signal_state == SIGNAL_FREE)
		{
			// ----- Signal is green -----

			// Check for signal state change
			if(DccSignal.nochange_cnt > (DccSignal.free_cnt+2))
				signal_state_new = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.free_cnt+2))
				signal_state_new = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.free_cnt+2))
				signal_state_new = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.free_cnt+2))
				signal_state_new = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.free_cnt+2))
				signal_state_new = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, TRUE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_NOCHANGE)
		{
			// ----- Signal is "no change" -----
			if(DccSignal.free_cnt > (DccSignal.nochange_cnt + 2))
				signal_state_new = SIGNAL_FREE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.nochange_cnt + 2))
				signal_state_new = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.nochange_cnt + 2))
				signal_state_new = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.nochange_cnt + 2))
				signal_state_new = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.nochange_cnt + 2))
				signal_state_new = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_90KMH)
		{
			// ----- Signal is 90km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp90kmh_cnt + 2))
				signal_state_new = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp90kmh_cnt + 2))
				signal_state_new = SIGNAL_NOCHANGE;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.sp90kmh_cnt + 2))
				signal_state_new = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.sp90kmh_cnt + 2))
				signal_state_new = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp90kmh_cnt + 2))
				signal_state_new = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, TRUE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_60KMH)
		{
			// ----- Signal is 60km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp60kmh_cnt + 2))
				signal_state_new = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp60kmh_cnt + 2))
				signal_state_new = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.sp60kmh_cnt + 2))
				signal_state_new = SIGNAL_90KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.sp60kmh_cnt + 2))
				signal_state_new = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp60kmh_cnt + 2))
				signal_state_new = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_40KMH)
		{
			// ----- Signal is 40km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp40kmh_cnt + 2))
				signal_state_new = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp40kmh_cnt + 2))
				signal_state_new = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.sp40kmh_cnt + 2))
				signal_state_new = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.sp40kmh_cnt + 2))
				signal_state_new = SIGNAL_60KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp40kmh_cnt + 2))
				signal_state_new = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, TRUE);
		}
		else
		{
			// ----- Signal is stop or unknown -----
			if(DccSignal.free_cnt > (DccSignal.stop_cnt + 2))
				signal_state_new = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.stop_cnt + 2))
				signal_state_new = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.stop_cnt + 2))
			{
				cnt_start(COUNTER_SIGNAL_FREEZE, (Mem.time_after_stop*4)/9);
				signal_state_new = SIGNAL_90KMH;
				DccInst.signal_state = signal_state_new;
			}
			else if(DccSignal.sp60kmh_cnt > (DccSignal.stop_cnt + 2))
			{
				cnt_start(COUNTER_SIGNAL_FREEZE, (Mem.time_after_stop*4)/6);
				signal_state_new = SIGNAL_60KMH;
				DccInst.signal_state = signal_state_new;
			}
			else if(DccSignal.sp40kmh_cnt > (DccSignal.stop_cnt + 2))
			{
				cnt_start(COUNTER_SIGNAL_FREEZE, Mem.time_after_stop);
				signal_state_new = SIGNAL_40KMH;
				DccInst.signal_state = signal_state_new;
			}
			else
			{
				cnt_start(COUNTER_SIGNAL_FREEZE, 0);
				signal_state_new = SIGNAL_STOP;
			}

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, TRUE);
		}

		if( (cnt_read(COUNTER_SIGNAL_FREEZE) == 0) ||
				(signal_state_new == SIGNAL_STOP) ||
				(signal_state_new == SIGNAL_40KMH))
		{
			DccInst.signal_state = signal_state_new;
		}
		else if(signal_state_new == SIGNAL_NOCHANGE)
		{
			DccInst.signal_state = signal_state_new;
			cnt_start(COUNTER_SIGNAL_FREEZE, 0);
		}
		else if(signal_state_new < DccInst.signal_state)
		{
			DccInst.signal_state = signal_state_new;
		}

		DccSignal.out_idx++;
		DccSignal.out_idx &= 0x07;
	}

}
