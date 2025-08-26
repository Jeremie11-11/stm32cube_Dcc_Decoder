/*
 * motor.c
 *
 *  Created on: 4 avr. 2024
 *      Author: J. Früh
 */

#include <dcc_protocol_rx.h>
#include <i_adc.h>
#include <i_timer.h>
#include <m_memory.h>
#include "motor.h"
#include <stdlib.h>

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

extern struct MEM_CONFIG_STRUCT Mem;
extern struct MEM_MOTOR_DATA_STRUCT memMotData;
extern DCC_INSTRUCTION_STRUCT DccInst;
extern ADC_STRUCT Adc;


MOTOR_STRUCT Motor = {
		.running = FALSE,
		.booting = TRUE
};


void mot_init(void)
{
	uint32_t i;

	for(i = 0; i <= 28; ++i)
	{
		Motor.Uref_cl[i] = 0 + i * 300;
		Motor.Uref_op[i] = 4000 + i * 500;
	}
}

static void mot_current_source(uint32_t state)
{
	if(state == ENABLE)
	{
		// Set PIN as push-pull to high state
		//GPIO_CONFIG_OUTPUT(PIN_CURRENT_EN);
		GPIO_WRITE(PIN_CURRENT_EN, TRUE);
	}
	else
	{
		// Set PIN as push-pull to low state
		//GPIO_CONFIG_OUTPUT(PIN_CURRENT_EN);
		GPIO_WRITE(PIN_CURRENT_EN, FALSE);
	}
}

// Updated in the main loop.
void mot_speed_update(void)
{
	int32_t old_speed = DccInst.actual_speed;

	// Leave if no valid drive instruction have been received
	if((DccInst.actual_dir != DIR_FORWARDS) && (DccInst.actual_dir != DIR_BACKWARDS))
		return;

	// Leave if the counter is not issued
	if(cnt_read(COUNTER_MOTOR_SPEED_UPDATE) > 0)
		return;
/*
	if(DccInst.emergency_stop != 0)
	{
		// Emergency stop: Stopping
		DccInst.target_speed = 0;

		// Reload the counter
		cnt_start(COUNTER_MOTOR_SPEED_UPDATE, 5);
	}
	else
	*/
	if(DccInst.signal_state == signal_red)
	{
		// RED signal: Stopping
		DccInst.target_speed = 0;

		// Reload the counter
		cnt_start(COUNTER_MOTOR_SPEED_UPDATE, 100);
	}
	else if(DccInst.signal_state == signal_orange)
	{
		// ORANGE signal: Speed limitation
		if(DccInst.dcc_target_speed >= 0)
		{
			if(DccInst.dcc_target_speed > 5)
				DccInst.target_speed = 5;
			else
				DccInst.target_speed = DccInst.dcc_target_speed;
		}
		else
		{
			if(DccInst.dcc_target_speed < -5)
				DccInst.target_speed = -5;
			else
				DccInst.target_speed = DccInst.dcc_target_speed;
		}

		// Reload the counter
		cnt_start(COUNTER_MOTOR_SPEED_UPDATE, 100);
	}
	else
	{
		// GREEN signal: Do not limit the speed
		DccInst.target_speed = DccInst.dcc_target_speed;

		// Reload the counter
		cnt_start(COUNTER_MOTOR_SPEED_UPDATE, 200);
	}

	if((Mem.motor_ctrl.e == CTRL_OPEN_LOOP) || (Mem.motor_ctrl.e == CTRL_OPEN_LOOP_5_PERCENT_PWM))
	{
		// ----- Open loop speed control -----
		if((DccInst.actual_speed * DccInst.target_speed > 0) || (DccInst.actual_speed == 0))
		{
				DccInst.actual_speed = DccInst.target_speed;
		}
		else
			// Changing direction
			DccInst.actual_speed = 0;
	}
	// ----- Close loop speed control -----
	else if(DccInst.target_speed > DccInst.actual_speed)
	{
		if((DccInst.actual_speed == 1) && ((Motor.Uemf_avg_mV > (Mem.Uref_min_start_mV-50)) || (Motor.starting >= 80)))
			// Acceleration only if the rotor is moving
			DccInst.actual_speed++;
		else if((DccInst.actual_speed == -1) && (Motor.Uemf_avg_mV <= (int32_t)Mem.Uemf_max_stop_mV))
			// Stopping only if the speed is slow
			DccInst.actual_speed++;
		else
			// Adapt speed
			DccInst.actual_speed++;
	}
	else if(DccInst.target_speed < DccInst.actual_speed)
	{
		if((DccInst.actual_speed == -1) && ((Motor.Uemf_avg_mV > (Mem.Uref_min_start_mV-50)) || (Motor.starting >= 80)))
			// Acceleration only if the rotor is moving
			DccInst.actual_speed--;
		else if((DccInst.actual_speed == 1) && (Motor.Uemf_avg_mV <= (int32_t)Mem.Uemf_max_stop_mV))
			// Stopping only if the speed is slow
			DccInst.actual_speed--;
		else
			// Adapt speed
			DccInst.actual_speed--;
	}

	if(DccInst.actual_speed == 0)
	{
		// Disable H-bridge
		tim_set_motor_bridge(DIR_STOPPED);

		if(Motor.running != FALSE)
		{
			// PWM control is now disable
			Motor.running = FALSE;

			mot_current_source(DISABLE);

			// Store start of the movement
			memMotData.Uin_mV[Motor.i]  = -1;
			memMotData.Imot_mA[Motor.i] = -1;
			memMotData.Uemf_mV[Motor.i] = -1;
			memMotData.Unew_mV[Motor.i] = -1;

			mem_write_motor();
		}
	}
	else if(old_speed == 0)
	{
		// ----- Preparing move -----

		// Skip acceleration ramp if booting
		if(Motor.booting == TRUE)
			DccInst.actual_speed = DccInst.target_speed;

		mot_current_source(ENABLE);

		if(DccInst.actual_speed > 0)
		{
			// ----- Starting forwards move -----
			// Update flag
			DccInst.actual_dir = DIR_FORWARDS;

			// Set lights (depending on direction)
			//dcc_update_functions();

			// Set H-brige
			tim_set_motor_bridge(DIR_FORWARDS);

			//if(Motor.Uint_mV < Mem.Ustart_fwd_mV)
				Motor.Uint_mV = Mem.Ustart_fwd_mV;
		}
		else
		{
			// ----- Starting backwards move -----
			// Update flag
			DccInst.actual_dir = DIR_BACKWARDS;

			// Set lights (depending on direction)
			//dcc_update_functions();

			// Set H-brige
			tim_set_motor_bridge(DIR_BACKWARDS);

			//if(Motor.Uint_mV < Mem.Ustart_bwd_mV)
				Motor.Uint_mV = Mem.Ustart_bwd_mV;
		}

		Motor.starting = 0;
		Motor.i = 0;

		// PWM control is now enable
		Motor.running = TRUE;
	}

	Motor.booting = FALSE;
}


void mot_pwm_update(void)
{
	int32_t Motor_Uemf2_mV=0;
	static int32_t var_p, var_i, var_p2, var_d;

	// Ensure motor is running
	if(Motor.running != TRUE)
		return;

	// Calculate real motor applied voltage
	Motor.Umot_mV = ((((int32_t)(Motor.ccr)) * Adc.Uin_mV) / PWM_MOTOR_PERIOD_CNT);

	if(DccInst.actual_dir == DIR_FORWARDS)
		Motor.Uemf_mV = Motor.Umot_mV - ((int32_t)Mem.Rcoil_fwd_mOhm * (int32_t)Adc.Ibridge_mA) / 1000;
	else
		Motor.Uemf_mV = Motor.Umot_mV - ((int32_t)Mem.Rcoil_bwd_mOhm * (int32_t)Adc.Ibridge_mA) / 1000;

	if(Mem.motor_ctrl.e == CTRL_OPEN_LOOP_5_PERCENT_PWM)
	{
		// ----- Open loop 5 percent PWM control -----
		// Mode used for motor identification, dimensioning and tests

		if(Motor.i >= 255)
			Motor.i = 0;

		// 5 percent PWM control (5% per speed)
		Motor.ccr = (PWM_MOTOR_PERIOD_CNT * (uint32_t)(abs(DccInst.actual_speed))) / 20 ;
		Motor.Unew_mV = ((int32_t)Adc.Uin_mV * (int32_t)Motor.ccr) / PWM_MOTOR_PERIOD_CNT;
	}
	else if(Mem.motor_ctrl.e == CTRL_OPEN_LOOP)
	{
		// ----- Open loop PWM control -----

		if(Motor.i >= 255)
			Motor.i = 0;

		// Set PWM on voltage reference (Following voltage table)
		Motor.Unew_mV = (int32_t)(Motor.Uref_op[abs(DccInst.actual_speed)]);
		Motor.ccr = (((uint32_t)(Motor.Unew_mV)) * PWM_MOTOR_PERIOD_CNT) / Adc.Uin_mV;
	}
	else
	{
		// ----- Close loop PWM control -----
		if(Motor.i == 0)
		{
			// ----- Init variable at start of move -----
			Motor.Umot_mV = 0;	// Effective voltage on motor
			Motor.Uemf_mV = 0;	// EMF voltage (indirect speed)
			Motor.Uemf_avg_mV = 0;	// Averaged EMF voltage
			Motor.Uemf_old_mV = 0;	// Old value from EMF voltage
			Motor.Uder_mV = 0;	//

			var_p = Mem.motor_p;
			var_i = Mem.motor_i;
			var_d = Mem.motor_d;
			var_p2 = 1U;
		}

		Motor_Uemf2_mV = (Motor.Uemf_old_mV + Motor.Uemf_mV)/2;

		Motor.Uemf_avg_mV = (Motor.Uemf_avg_mV*7 + Motor.Uemf_mV)/8;

		// REF
		Motor.Uref_mV = Motor.Uref_cl[abs(DccInst.actual_speed)];
		if((Motor.starting < 80) && (Motor.Uref_mV < Mem.Uref_min_start_mV))
			Motor.Uref_mV = Mem.Uref_min_start_mV;

		if(Motor.Uref_mV < Mem.Uref_min_mV)
			Motor.Uref_mV = Mem.Uref_min_mV;


		if((abs(DccInst.actual_speed) == 1) && (Motor.starting >= 40))
		{
			if(Motor.Uemf_mV < (Mem.Uref_min_start_mV-50))
			{
				var_p2 = var_p2 + (var_p2 / 2) + 1;
				if(var_p2 > 256)
					var_p2 = 256;
			}
			else
			{
				var_p2 -= 4;
				//var_p2 = var_p2 - (var_p2 / 16) - 1;
				if(var_p2 < 1)
					var_p2 = 1;
			}

			//Motor.Ustart = (((Motor.Uref_mV - Motor.Uemf_mV) * var_p2) / 64);
			Motor.Ustart = (((Motor.Uref_mV - Motor_Uemf2_mV) * var_p2) / 64);


			Motor.Uder_mV = ((Motor.Uemf_old_mV - Motor.Uemf_mV) * var_d) / 64;
		}
		else if(Motor.starting == 0)
		{
			Motor.Ustart = 8000;
			Motor.Uder_mV = 0;
		}
		else
		{
			Motor.Ustart = 0;

			Motor.Uder_mV = 0;
		}

		if(Motor.Uder_mV > 0)
			Motor.Uder_mV = 0;
		else if(Motor.Uder_mV < -5000)
			Motor.Uder_mV = -5000;
		Motor.Uemf_old_mV = Motor.Uemf_mV;

		Motor.Uint_mV += ((Motor.Uref_mV - Motor.Uemf_mV) * var_i) / 64;
		if(Motor.Uint_mV > 20000)
			Motor.Uint_mV = 20000;
		else if(Motor.Uint_mV < 3000)
			Motor.Uint_mV = 3000;

		Motor.Unew_mV = Motor.Ustart;
		Motor.Unew_mV += ((Motor.Uref_mV - Motor.Uemf_mV) * var_p) / 64;
		Motor.Unew_mV += Motor.Uint_mV;
		Motor.Unew_mV += Motor.Uder_mV;

		//Motor.Unew_mV = (((Motor.Uref_mV - Motor.Uemf_mV) * var_p) / 64) + Motor.Uint_mV + Motor.Uder_mV + Motor.Ustart;
		if(Motor.Unew_mV < (int32_t) (Mem.Umin_mV))
			Motor.Unew_mV = (int32_t) (Mem.Umin_mV);

		Motor.ccr = (Motor.Unew_mV * PWM_MOTOR_PERIOD_CNT) / Adc.Uin_mV;

	}

	// PWM limitation between 0.02 and 0.98 duty-cycle
	if(Motor.ccr > PWM_MAX)
		Motor.ccr = PWM_MAX;
	else if(Motor.ccr <= PWM_MIN)
		Motor.ccr = PWM_MIN;

	//tim_set_motor_pwm(DccInst.actual_dir, Motor.ccr);

	// Both channel are updated, but only one is enabled.
	TIM1->CCR1 = Motor.ccr;
	TIM1->CCR2 = Motor.ccr;

	//
	if(Motor.starting < 1000)
		Motor.starting++;

	// Debug
	memMotData.Uin_mV[Motor.i]  = Adc.Uin_mV;
	memMotData.Imot_mA[Motor.i] = Adc.Ibridge_mA;
	memMotData.Uemf_mV[Motor.i] = Motor.Uemf_mV;
	memMotData.Unew_mV[Motor.i] = Motor.Unew_mV;
	if(Motor.i < 255)
		Motor.i++;
}

