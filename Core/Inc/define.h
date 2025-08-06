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
	DIR_FORWARDS = 0,
	DIR_BACKWARDS = 1,
	DIR_STOPPED = 2
}dir_t;

typedef enum{
	CTRL_OPEN_LOOP = 0,
	CTRL_CLOSE_LOOP = 1
}motor_ctrl_t;


typedef enum{
	front_light 	= 1 << 0,
	cab_light 		= 1 << 1,
	rear_light 		= 1 << 2,
	opt_light			= 1 << 3
}light_t;


#endif /* INC_DEFINE_H_ */
