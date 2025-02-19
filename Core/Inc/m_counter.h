/*
 * counter.h
 *
 *  Created on: Oct 18, 2024
 *      Author: J. Früh
 */

#ifndef INC_M_COUNTER_H_
#define INC_M_COUNTER_H_

#include "main.h"

// List of the counter used in code
typedef enum{
    COUNTER_UNIVERSAL1 = 0,
    COUNTER_UNIVERSAL2,

		COUNTER_MOTOR_SPEED_UPDATE,
		COUNTER_MOTOR_SPEED1_TIMEOUT,
		COUNTER_MOTOR_SLOW_START,
		COUNTER_DCC_RX_TIMEOUT,
		COUNTER_LED_RED,

		NBR_COUNTERS	// Variable used to get the number of declared counter
} counters_enum;

// Structure of the counters
typedef struct{
	uint32_t time[NBR_COUNTERS];
} COUNTERS_STRUCT;


void cnt_start(counters_enum cnt_idx, uint32_t time_ms);

void cnt_start_if_not(counters_enum cnt_idx, uint32_t time_ms);

uint32_t cnt_read(counters_enum cnt_idx);

void cnt_update();

#endif /* INC_M_COUNTER_H_ */
