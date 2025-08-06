/*
 * error_handler.c
 *
 *  Created on: Dec 11, 2024
 *      Author: J. Früh
 */

#include "debug.h"


void debug_leds_update(void)
{
	// LED_STATUS_RED
	// 1000ms: CPU reset
	// 20ms: Errors
	if(cnt_read(COUNTER_LED_STATUS_RED) == 0)
	{
		GPIO_WRITE(LED2_RED, FALSE);
	}

	// LED_STATUS_GREEN blinking
	// 5Hz: DCC Rx communication successful
	// 0.5Hz: No DCC Rx communication
	if(cnt_read(COUNTER_DCC_RX_TIMEOUT) == 0)
	{
		debug_toggle_led_status_green(LED_DCC_COM, 2000);
	}

}

// Red LED is used when booting, memory writing and errors
void debug_set_led_status_red(led_event_t event, uint32_t time_ms)
{
	if(LEDS_EVENT_REGISTER & event)
	{
		GPIO_WRITE(LED2_RED, TRUE);
		cnt_start_or_extend(COUNTER_LED_STATUS_RED, time_ms);
	}
}


void debug_set_led_status_green(led_event_t event, uint32_t time_ms)
{
	if(LEDS_EVENT_REGISTER & event)
	{
		GPIO_WRITE(LED1_GREEN, TRUE);
		cnt_start(COUNTER_DCC_RX_TIMEOUT, time_ms);
	}
}


void debug_toggle_led_status_green(led_event_t event, uint32_t time_ms)
{
	if(GPIO_READ(LED1_GREEN) == TRUE)
	{
		GPIO_WRITE(LED1_GREEN, FALSE);
	}
	else
	{
		if(LEDS_EVENT_REGISTER & event)
			GPIO_WRITE(LED1_GREEN, TRUE);
	}
	cnt_start(COUNTER_DCC_RX_TIMEOUT, time_ms);
}



