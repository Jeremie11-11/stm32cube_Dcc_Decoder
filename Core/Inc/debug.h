/*
 * error_handler.h
 *
 *  Created on: Dec 11, 2024
 *      Author: J. Früh
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "main.h"

typedef enum{
    LED_BOOT_EVENT 			= 1<<0,
    LED_DCC_COM 				= 1<<1,
    LED_DCC_COM_ERRORS 	= 1<<2,
		LED_MEM_WRITE 			= 1<<3
} led_event_t;

#define LEDS_EVENT_REGISTER (LED_BOOT_EVENT | LED_DCC_COM | LED_DCC_COM_ERRORS | LED_MEM_WRITE)

void debug_leds_update(void);

void debug_set_led_status_red(led_event_t event, uint32_t time_ms);

void debug_set_led_status_green(led_event_t event, uint32_t time_ms);

void debug_toggle_led_status_green(led_event_t event, uint32_t time_ms);

#endif /* INC_DEBUG_H_ */
