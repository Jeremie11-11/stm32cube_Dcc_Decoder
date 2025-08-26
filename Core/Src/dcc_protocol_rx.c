/*
 * dcc_decoder.c
 *
 *  Created on: 28 mars 2024
 *      Author: J. Früh
 */

#include <dcc_physical_layer.h>
#include <dcc_protocol_rx.h>
#include <i_adc.h>
#include <i_timer.h>
#include <m_memory.h>


extern DCC_PROTOCOL_STRUCT DccRx;
extern DCC_DEBUG_STRUCT DccDebug;
extern struct MEM_CONFIG_STRUCT Mem;
extern ADC_STRUCT Adc;

DCC_INSTRUCTION_STRUCT DccInst = {
		.actual_dir = DIR_DEFAULT_VALUE,
		.dcc_target_speed = 0,
		.target_speed = 0,
		.actual_speed = 0,
		.signal_state = signal_green
};


uint32_t decoded_address_match(DCC_MESSAGE_STRUCT *msg, uint8_t *buffer)
{
	if(buffer[0] < 0x80)
	{
		// 7 bit addresses
		msg->addr = buffer[0];
	}
	else
	{
		// 16 bit address
		msg->addr = (buffer[1] << 8) + buffer[0];
	}

	if((msg->addr == Mem.address) || (msg->addr == ADDR_BROADCAST))
		return TRUE;
	else
		return FALSE;
}

// 0: Same content als previous message
// 1: New content
uint32_t msg_with_new_content(uint8_t *buffer, uint8_t len)
{
	static uint8_t last_msg[10], last_len=0;

	if((len != last_len) || (memcmp(buffer, last_msg, len) != 0))
	{
		// ----- Difference recognized -----

		// Update last message
		memcpy(last_msg, buffer, len);
		last_len = len;

		return TRUE;
	}
	return FALSE;
}

void decoding_msg_content(DCC_MESSAGE_STRUCT *msg, uint8_t *buffer, uint8_t len)
{
	uint8_t inst_speed=0;

	if(msg->addr < 0x80)
	{
		// With 1 byte address
		msg->nb_data = len - 2; //len - 1 - 1

		msg->inst = buffer[1] >> 5;

		inst_speed = ((buffer[1] & 0x0F) << 1) + ((buffer[1] & 0x10) >> 4);
		msg->fct = buffer[2];

		msg->mem_idx = buffer[2];
		msg->mem_data = buffer[3];
	}
	else
	{
		// With 2 bytes address
		msg->nb_data = len - 3; //len - 2 - 1

		msg->inst = buffer[2] >> 5;

		inst_speed = ((buffer[2] & 0x0F) << 1) + ((buffer[2] & 0x10) >> 4);
		msg->fct = buffer[3];

		msg->mem_idx = buffer[3];
		msg->mem_data = buffer[4];
	}

	if(inst_speed <= 1)
	{
		msg->emergency = 0;
		msg->speed = 0;
	}
	else if(inst_speed <= 3)
	{
		msg->emergency = 1;
		msg->speed = 0;
	}
	else
	{
		msg->emergency = 0;
		msg->speed = inst_speed - 3;
	}
}




void dcc_check_for_new_messages(void)
//void dcc_msg_decoder(void)
{
	DCC_MESSAGE_STRUCT msg;
	uint8_t *buffer, len;

	// Check for new message
	if(DccRx.msg_out_i == DccRx.msg_in_i)
		return;

	// ----- New message received -----

	// Led green blinking(5Hz) when communication is working
	if((DccDebug.recieved_msg%40) == 0)
		debug_toggle_led_status_green(LED_DCC_COM, 2000);

	buffer = DccRx.msg[DccRx.msg_out_i].data;
	len = DccRx.msg[DccRx.msg_out_i].len;

	// Ensure that the address match
	if(decoded_address_match(&msg, buffer) == FALSE)
	{
		// Address is not matching
		DccRx.msg_out_i = (DccRx.msg_out_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);
		return;
	}

	// Led green blinking(20ms ON) when dedicated message received
	DccDebug.recieved_msg = 1;
	debug_set_led_status_green(LED_DCC_COM, 20);

	// Check for new content
	if(msg_with_new_content(buffer, len) == FALSE)
	{
		// Message content is still the same
		DccRx.msg_out_i = (DccRx.msg_out_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);
		return;
	}

	// ----- Received message with new contents -----

	decoding_msg_content(&msg, buffer, len);

	if(msg.inst == speed_dir_inst_forward)
	{
		if(msg.nb_data >= 1) {
			DccInst.dcc_target_speed = msg.speed;
			DccInst.emergency_stop = msg.emergency;
		}

		if(msg.nb_data >= 2)
			DccInst.functions = msg.fct;

		if((DccInst.actual_dir != DIR_FORWARDS) && (DccInst.actual_dir != DIR_BACKWARDS))
			DccInst.actual_dir = DIR_FORWARDS;

		// Update direction if motor is stopped
		if((DccInst.dcc_target_speed == 0) && (DccInst.actual_speed == 0))
			DccInst.actual_dir = DIR_FORWARDS;

		//dcc_update_functions();
	}
	else if(msg.inst == speed_dir_inst_reverse)
	{
		if(msg.nb_data >= 1) {
			DccInst.dcc_target_speed = -msg.speed;
			DccInst.emergency_stop = msg.emergency;
		}

		if(msg.nb_data >= 2)
			DccInst.functions = msg.fct;

		// Update direction if motor is stopped
		if((DccInst.actual_dir != DIR_FORWARDS) && (DccInst.actual_dir != DIR_BACKWARDS))
			DccInst.actual_dir = DIR_BACKWARDS;

		// Update direction if motor is stopped
		if((DccInst.dcc_target_speed == 0) && (DccInst.actual_speed == 0))
			DccInst.actual_dir = DIR_BACKWARDS;

		//dcc_update_functions();
	}
	else if(msg.inst == config_variable_inst)
	{
		if(msg.nb_data == 6)
		{
			// ----- Store configuration into flash memory -----
			uint8_t mem_idx = buffer[2] * 4;

			if(mem_idx < sizeof(Mem))
			{
				memcpy(((uint8_t *)&Mem) + mem_idx, &buffer[3], 4);

				mem_write_config();
				mem_read_config();

				debug_set_led_status_red(LED_MEM_WRITE, 50);
			}
		}
	}

	DccRx.msg_out_i = (DccRx.msg_out_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);

}
/*
void dcc_update_functions(void)
{
	if(Adc.under_voltage == TRUE)
	{
		GPIO_WRITE(FRONT_LIGHT, 0);
		GPIO_WRITE(REAR_LIGHT, 0);
		GPIO_WRITE(CAB_LIGHT, 0);
		GPIO_WRITE(OPT_LIGHT, 0);

	}
	else
	{
		if(DccInst.actual_dir == DIR_FORWARDS)
		{
			GPIO_WRITE(FRONT_LIGHT, (DccInst.functions >> 0) & 0x01);
			GPIO_WRITE(REAR_LIGHT, (DccInst.functions >> 2) & 0x01);

			tim_set_light(front_light, DccInst.functions);
			tim_set_light(rear_light, DccInst.functions);
		}
		else
		{
			GPIO_WRITE(FRONT_LIGHT, (DccInst.functions >> 2) & 0x01);
			GPIO_WRITE(REAR_LIGHT, (DccInst.functions >> 0) & 0x01);

			tim_set_light(front_light, DccInst.functions >> 2);
			tim_set_light(rear_light, DccInst.functions << 2);
		}

		GPIO_WRITE(CAB_LIGHT, (DccInst.functions >> 1) & 0x01);
		GPIO_WRITE(OPT_LIGHT, (DccInst.functions >> 3) & 0x01);

		tim_set_light(cab_light, DccInst.functions);
		tim_set_light(opt_light, DccInst.functions);
	}
}
*/

