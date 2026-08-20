/*
 * dcc_signal.c
 *
 *  Created on: Jan 22, 2026
 *      Author: J. Früh
 */

#include <dcc_signal.h>

#include <dcc_protocol_rx.h>


DCC_SIGNAL_STRUCT DccSignal;

extern DCC_INSTRUCTION_STRUCT DccInst;



static const int16_t signal_speed_limit[] =
{
	[SIGNAL_STOP] = 0,
	[SIGNAL_40KMH] = 40,
	[SIGNAL_60KMH] = 60,
	[SIGNAL_90KMH] = 90,
	[SIGNAL_FREE] = 320
};

typedef struct
{
	uint8_t length[DCC_TIMEOUT_TIME_COUNT];
	signal_state_t signal_state;
} DCC_TIMEOUT_PATTERN_STRUCT;

static const DCC_TIMEOUT_PATTERN_STRUCT dcc_timeout_pattern[] =
{
	{{1, 1, 1, 1, 1, 1, 1, 1}, SIGNAL_NOCHANGE},
	{{1, 1, 1, 1, 1, 3, 1, 1}, SIGNAL_STOP},
	{{1, 1, 1, 3, 1, 1, 1, 1}, SIGNAL_60KMH},
	{{1, 1, 1, 5, 1, 1, 1, 1}, SIGNAL_40KMH},
	{{1, 3, 1, 1, 1, 1, 1, 1}, SIGNAL_90KMH},
	{{1, 3, 1, 3, 1, 1, 1, 1}, SIGNAL_NOCHANGE},
	{{1, 5, 1, 1, 1, 1, 1, 1}, SIGNAL_FREE}
};


void signal_speed_limit_update()
{
	// Update speed limit
	if(DccInst.signal_state != SIGNAL_NOCHANGE)
		DccInst.speed_limit = signal_speed_limit[DccInst.signal_state];
	else if (DccInst.speed_limit < signal_speed_limit[SIGNAL_40KMH])
		DccInst.speed_limit = signal_speed_limit[SIGNAL_40KMH];
}


static void signal_filter_update(signal_state_t detected_state)
{
	switch(detected_state)
	{
		case SIGNAL_FREE:
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
			break;

		case SIGNAL_NOCHANGE:
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
			break;

		case SIGNAL_90KMH:
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
			break;

		case SIGNAL_60KMH:
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
			break;

		case SIGNAL_40KMH:
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
			break;

		case SIGNAL_STOP:
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
			break;

		default:
			break;
	}

		if(DccInst.signal_state == SIGNAL_FREE)
		{
			// ----- Signal is green -----

			// Check for signal state change
			if(DccSignal.nochange_cnt > (DccSignal.free_cnt+2))
				DccInst.signal_state = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.free_cnt+2))
				DccInst.signal_state = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.free_cnt+2))
				DccInst.signal_state = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.free_cnt+2))
				DccInst.signal_state = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.free_cnt+2))
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, TRUE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_NOCHANGE)
		{
			// ----- Signal is "no change" -----
			if(DccSignal.free_cnt > (DccSignal.nochange_cnt + 2))
				DccInst.signal_state = SIGNAL_FREE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.nochange_cnt + 2))
				DccInst.signal_state = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.nochange_cnt + 2))
				DccInst.signal_state = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.nochange_cnt + 2))
				DccInst.signal_state = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.nochange_cnt + 2))
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_90KMH)
		{
			// ----- Signal is 90km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp90kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp90kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_NOCHANGE;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.sp90kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.sp90kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp90kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, TRUE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_60KMH)
		{
			// ----- Signal is 60km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp60kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp60kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.sp60kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_90KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.sp60kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_40KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp60kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, FALSE);
		}
		else if(DccInst.signal_state == SIGNAL_40KMH)
		{
			// ----- Signal is 40km/h -----
			if(DccSignal.free_cnt > (DccSignal.sp40kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.sp40kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.sp40kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.sp40kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_60KMH;
			else if(DccSignal.stop_cnt > (DccSignal.sp40kmh_cnt + 2))
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, TRUE);
			GPIO_WRITE(TEST_PIN4, TRUE);
		}
		else
		{
			// ----- Signal is stop or unknown -----
			if(DccSignal.free_cnt > (DccSignal.stop_cnt + 2))
				DccInst.signal_state = SIGNAL_FREE;
			else if(DccSignal.nochange_cnt > (DccSignal.stop_cnt + 2))
				DccInst.signal_state = SIGNAL_NOCHANGE;
			else if(DccSignal.sp90kmh_cnt > (DccSignal.stop_cnt + 2))
				DccInst.signal_state = SIGNAL_90KMH;
			else if(DccSignal.sp60kmh_cnt > (DccSignal.stop_cnt + 2))
				DccInst.signal_state = SIGNAL_60KMH;
			else if(DccSignal.sp40kmh_cnt > (DccSignal.stop_cnt + 2))
				DccInst.signal_state = SIGNAL_40KMH;
			else
				DccInst.signal_state = SIGNAL_STOP;

			GPIO_WRITE(TEST_PIN2, FALSE);
			GPIO_WRITE(TEST_PIN3, FALSE);
			GPIO_WRITE(TEST_PIN4, TRUE);
		}
	}


void dcc_signal_update(void)
{
	while(DccTimeout.out_idx != DccTimeout.in_idx)
	{
		for(uint32_t i = 0; i < (sizeof(dcc_timeout_pattern) / sizeof(dcc_timeout_pattern[0])); i++)
		{
			uint8_t pattern_match = TRUE;

			for(uint32_t time_idx = 0; time_idx < DCC_TIMEOUT_TIME_COUNT; time_idx++)
			{
				if(DccTimeout.time[DccTimeout.out_idx][time_idx] != dcc_timeout_pattern[i].length[time_idx])
				{
					pattern_match = FALSE;
					break;
				}
			}

			if(pattern_match == TRUE)
			{
				signal_filter_update(dcc_timeout_pattern[i].signal_state);
				break;
			}
		}

		DccTimeout.out_idx = (DccTimeout.out_idx + 1U) & DCC_TIMEOUT_QUEUE_MASK;
	}
}


void signal_update()
{
	while(DccSignal.out_idx != DccSignal.in_idx)
	{
		uint32_t timeout = DccSignal.timeout_tab[DccSignal.out_idx];
		signal_state_t detected_state = (signal_state_t)0xFF;

		if(timeout < TIMEOUT__THRES_FREE)
			detected_state = SIGNAL_FREE;
		else if(timeout < TIMEOUT__THRES_NOCHANGE)
			detected_state = SIGNAL_NOCHANGE;
		else if(timeout < TIMEOUT__THRES_90KMH)
			detected_state = SIGNAL_90KMH;
		else if(timeout < TIMEOUT__THRES_60KMH)
			detected_state = SIGNAL_60KMH;
		else if(timeout < TIMEOUT__THRES_40KMH)
			detected_state = SIGNAL_40KMH;
		else if(timeout < TIMEOUT__THRES_STOP)
			detected_state = SIGNAL_STOP;

		signal_filter_update(detected_state);

		DccSignal.out_idx++;
		DccSignal.out_idx &= 0x07;
	}

}
