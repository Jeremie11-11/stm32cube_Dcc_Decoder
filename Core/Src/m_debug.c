/*
 * error_handler.c
 *
 *  Created on: Dec 11, 2024
 *      Author: J. Früh
 */

#include <m_debug.h>
#include <m_memory.h>


extern struct MEM_CONFIG_STRUCT Mem;


void dbg_leds_update(void)
{
	// LED2_RED
	// 1000ms: CPU reset
	// 20ms: Errors
	if(cnt_read(COUNTER_LED_RED) == 0)
	{
		GPIO_WRITE(LED2_RED, FALSE);
	}

	// LED1_GREEN blinking
	// 5Hz: DCC Rx communication successful
	// 0.5Hz: No DCC Rx communication
	if(cnt_read(COUNTER_DCC_RX_TIMEOUT) == 0)
	{
		dbg_toggle_green_led(LED_DCC_COM, 2000);
	}
}

void dbg_set_green_led(led_event_t event, uint32_t time_ms)
{
	if(Mem.event_ctrl.u32 & event)
		GPIO_WRITE(LED1_GREEN, TRUE);
	cnt_start(COUNTER_DCC_RX_TIMEOUT, time_ms);
}

void dbg_toggle_green_led(led_event_t event, uint32_t time_ms)
{
	if(GPIO_READ(LED1_GREEN) == TRUE)
	{
		GPIO_WRITE(LED1_GREEN, FALSE);
	}
	else
	{
		if(Mem.event_ctrl.u32 & event)
			GPIO_WRITE(LED1_GREEN, TRUE);
	}
	cnt_start(COUNTER_DCC_RX_TIMEOUT, time_ms);
}

void dbg_set_error_led(led_event_t event, uint32_t time_ms)
{
	if(Mem.event_ctrl.u32 & event)
	{
		GPIO_WRITE(LED2_RED, TRUE);
		cnt_start_if_not(COUNTER_LED_RED, time_ms);
	}
}


