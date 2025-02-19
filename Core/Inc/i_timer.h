/*
 * timer.h
 *
 *  Created on: Feb 9, 2023
 *      Author: J. Frueh
 */

#ifndef INC_I_TIMER_H_
#define INC_I_TIMER_H_

#include "main.h"


#define PWM_MOTOR_PERIOD_CNT 2000
#define PWM_MAX (PWM_MOTOR_PERIOD_CNT * 0.98)
#define PWM_MIN (PWM_MOTOR_PERIOD_CNT * 0.02)

#define PWM_LIGHT_PERIOD_CNT 1000

void tim_init(void);

void tim_set_motor_bridge(dir_t direction);

void tim_set_motor_pwm(dir_t direction, uint32_t pwm);

void tim_set_light(light_t light, uint8_t function);

void tim_BusyWait_us(uint32_t us);


#endif /* INC_I_TIMER_H_ */
