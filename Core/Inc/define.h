/*
 * define.h
 *
 *  Created on: Jun 11, 2025
 *      Author: J. Früh
 */

#ifndef INC_DEFINE_H_
#define INC_DEFINE_H_


#define FALSE 0U
#define TRUE 1U
#define DISABLE 0U
#define ENABLE 1U


typedef enum{
	_DISABLE = 0,
	_ENABLE = 1
} enable_t;


typedef enum{
	signal_red = 0,
	signal_orange = 1,
	signal_green = 2
}signal_state_t;


typedef enum{
	DIR_FORWARDS = 0,
	DIR_BACKWARDS = 1,
	DIR_STOPPED = 2,
	DIR_DEFAULT_VALUE = 3
}dir_t;

typedef enum{
	CTRL_OPEN_LOOP = 0,
	CTRL_CLOSE_LOOP = 1,
	CTRL_CLOSE_LOOP_WITH_HALL = 2,
	CTRL_OPEN_LOOP_5_PERCENT_PWM = 3,
	CTRL_ADC_MEASUREMENT = 4	// To be defined

}motor_ctrl_t;


typedef enum{
	front_light 	= 1 << 0,
	cab_light 		= 1 << 1,
	rear_light 		= 1 << 2,
	opt_light			= 1 << 3
}light_t;


#endif /* INC_DEFINE_H_ */
