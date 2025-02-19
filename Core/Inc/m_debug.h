/*
 * error_handler.h
 *
 *  Created on: Dec 11, 2024
 *      Author: J. Früh
 */

#ifndef INC_M_DEBUG_H_
#define INC_M_DEBUG_H_

#include "main.h"

typedef enum{
    LED_BOOT_EVENT 			= 1<<0,
    LED_DCC_COM 				= 1<<1,
    LED_DCC_COM_ERRORS 	= 1<<2,
		LED_MEM_WRITE 			= 1<<3
} led_event_t;


void dbg_leds_update(void);

void dbg_set_green_led(led_event_t event, uint32_t time_ms);

void dbg_toggle_green_led(led_event_t event, uint32_t time_ms);

void dbg_set_error_led(led_event_t event, uint32_t time_ms);

#endif /* INC_M_DEBUG_H_ */
