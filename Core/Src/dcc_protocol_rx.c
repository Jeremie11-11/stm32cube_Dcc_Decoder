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


void dcc_init(uint32_t backup_valid)
{
	uint32_t encoded_value;

	dcc_physical_layer_init();

	if(backup_valid == TRUE)
	{
		encoded_value = mem_read_backup_register(BACKUP_IDX__DCC_INFO_0);
		DccInst.signal_state = (encoded_value >> 24) & 0x000000FF;
		DccInst.dcc_target_speed = (encoded_value >> 16) & 0x000000FF;
		DccInst.actual_speed = (encoded_value >> 8) & 0x000000FF;
		DccInst.actual_dir = (encoded_value >> 0) & 0x000000FF;

		encoded_value = mem_read_backup_register(BACKUP_IDX__DCC_INFO_1);
		DccInst.functions = (encoded_value >> 24) & 0x000000FF;
	}
	else
	{
		DccInst.signal_state = signal_green;
		DccInst.dcc_target_speed = 0;
		DccInst.actual_speed = 0;
		DccInst.actual_dir = DIR_DEFAULT_VALUE;
		DccInst.functions = 0x80;
	}
}


void dcc_backup_info(void)
{
	uint32_t encoded_value;

	encoded_value = (DccInst.signal_state << 24) | (DccInst.dcc_target_speed << 16) | (DccInst.actual_speed << 8) | (DccInst.actual_dir);
	mem_write_backup_register(BACKUP_IDX__DCC_INFO_0, encoded_value);

	encoded_value = (DccInst.functions << 24);
	mem_write_backup_register(BACKUP_IDX__DCC_INFO_1, encoded_value);
}


uint32_t decoded_address_match(DCC_MESSAGE_STRUCT *msg, uint8_t *buffer, uint8_t len)
{
	if(buffer[0] < 128)
	{
		// ---------- 7 bits address ----------
		// 7 bit addresses (stored in the 1st byte)
		msg->addr = buffer[0];

		// Number of available date in message (Remove address(1 byte) and CRC(1 byte))
		msg->nb_data = len-2;

		if(msg->nb_data > 0)
			memcpy(msg->data, &buffer[1], msg->nb_data);
		else
			return FALSE;
	}
	else if((buffer[0] >= 192) && (buffer[0] <= 231))
	{
		// ---------- 14 bits address ----------
		// 14 bit address (up to 0x27FF = 10239)
		msg->addr = ((buffer[0] & 0x3F) << 8) + buffer[1];

		// Number of available date in message (Remove address(2 byte) and CRC(1 byte))
		msg->nb_data = len-3;

		if(msg->nb_data > 0)
			memcpy(msg->data, &buffer[2], msg->nb_data);
		else
			return FALSE;
	}
	else
	{
		// Address format not recognized
		return FALSE;
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

// 1. Check for a new message in the buffer
// 2. Check the message recipient (address)
// 3. Check for new contents
// 4. Decode the protocol
// Called in main loop
void dcc_check_for_new_messages(void)
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
	if(decoded_address_match(&msg, buffer, len) == FALSE)
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

	// ----------------------------------------------------------------------
	// --------------------- Message with new contents ----------------------
	// ----------------------------------------------------------------------
	// Message with right format, correct CRC and address match

	// Get message instruction
	msg.inst = msg.data[0] >> 5;

	// Decoding ...

	if(msg.inst == config_variable_inst)
	{
		// ---------- MSG_DCC_MEMORY ----------
		if(msg.nb_data == 6)
		{
			uint8_t mem_idx = msg.data[1] * 4;

			if(mem_idx < sizeof(Mem))
			{
				// Store the value into the RAM memory
				memcpy(((uint8_t *)&Mem) + mem_idx, &msg.data[2], 4);

				// Store RAM configuration into flash memory
				mem_write_config();
				mem_read_config();

				// LED blink one time in red
				debug_set_led_status_red(LED_MEM_WRITE, 50);
			}
		}
	}
	else if(msg.inst == function_group_one_inst)
	{
		// ---------- MSG_DCC_FUNCTION ----------

	}
	else if(msg.inst == speed_dir_inst_forward)
	{
		// ---------- MSG_DCC_SPEED (FORWARDS) ----------
		if(msg.nb_data >= 1)
		{
			int32_t speed = ((msg.data[0] & 0x0F) << 1) + ((msg.data[0] & 0x10) >> 4) - 3;

			if((speed == -1) || (speed == 0))
				DccInst.emergency_stop = TRUE;
			else
				DccInst.emergency_stop = FALSE;

			if(speed < 0)
				speed = 0;

			DccInst.dcc_target_speed = speed;
		}

		if(msg.nb_data >= 2)
			DccInst.functions = msg.data[1];

		// Update direction if not initialized (for reboot in run)
		if((DccInst.actual_dir != DIR_FORWARDS) && (DccInst.actual_dir != DIR_BACKWARDS))
			DccInst.actual_dir = DIR_FORWARDS;

		// Update direction if motor is stopped
		if((DccInst.dcc_target_speed == 0) && (DccInst.actual_speed == 0))
			DccInst.actual_dir = DIR_FORWARDS;

		dcc_update_functions();
	}
	else if(msg.inst == speed_dir_inst_reverse)
	{
		// ---------- MSG_DCC_SPEED (BACKWARDS) ----------
		if(msg.nb_data >= 1)
		{
			int32_t speed = ((msg.data[0] & 0x0F) << 1) + ((msg.data[0] & 0x10) >> 4) - 3;

			if((speed == -1) || (speed == 0))
				DccInst.emergency_stop = TRUE;
			else
				DccInst.emergency_stop = FALSE;

			if(speed < 0)
				speed = 0;

			DccInst.dcc_target_speed = -speed;
		}

		if(msg.nb_data >= 2)
			DccInst.functions = msg.data[1];

		// Update direction if not initialized (for reboot in run)
		if((DccInst.actual_dir != DIR_FORWARDS) && (DccInst.actual_dir != DIR_BACKWARDS))
			DccInst.actual_dir = DIR_BACKWARDS;

		// Update direction if motor is stopped
		if((DccInst.dcc_target_speed == 0) && (DccInst.actual_speed == 0))
			DccInst.actual_dir = DIR_BACKWARDS;

		dcc_update_functions();
	}

	// Next message
	DccRx.msg_out_i = (DccRx.msg_out_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);
}

void dcc_update_functions(void)
{
	if(DccInst.actual_dir == DIR_FORWARDS)
	{
		GPIO_WRITE(FRONT_LIGHT, (DccInst.functions >> 0) & 0x01);
		GPIO_WRITE(REAR_LIGHT, (DccInst.functions >> 2) & 0x01);
	}
	else
	{
		GPIO_WRITE(FRONT_LIGHT, (DccInst.functions >> 2) & 0x01);
		GPIO_WRITE(REAR_LIGHT, (DccInst.functions >> 0) & 0x01);
	}

	GPIO_WRITE(CAB_LIGHT, (DccInst.functions >> 1) & 0x01);
	GPIO_WRITE(OPT_LIGHT, (DccInst.functions >> 3) & 0x01);

	//tim_set_light(opt_light, DccInst.functions);

}


