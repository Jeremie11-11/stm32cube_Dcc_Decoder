/*
 * motor.h
 *
 *  Created on: 4 avr. 2024
 *      Author: J. Früh
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"


typedef struct{
	uint32_t ccr;
	int32_t Unew_mV;
	int32_t Umot_mV;
	int32_t Uemf_avg_mV;
	int32_t Uemf_mV;
	int32_t Uemf_old_mV;
	uint16_t Uref_mV;
	int32_t Uint_mV;
	int32_t Uder_mV;
	int32_t Ustart;
	uint16_t current;
	uint32_t i;
	uint32_t use_ref;
	int32_t starting;
	int32_t step;
	int32_t running;
	int32_t use_backup_register;
	int32_t init_current[100];
	uint16_t Uref_cl[29];
	uint16_t Uref_op[29];
} MOTOR_STRUCT;

void mot_init(uint32_t backup_valid);

void mot_speed_update(void);

void mot_pwm_update(void);

#endif /* INC_MOTOR_H_ */
