/*
 * memory.c
 *
 *  Created on: Apr 3, 2024
 *      Author: J. Frühie
 */

#include <dcc_protocol_rx.h>
#include <m_memory.h>
#include <string.h>
//#include "define.h"

extern RTC_HandleTypeDef hrtc;


struct MEM_CONFIG_STRUCT Mem;
struct MEM_MOTOR_STATUS_STRUCT memMotStat;
struct MEM_MOTOR_DATA_STRUCT memMotData;

extern DCC_INSTRUCTION_STRUCT DccInst;

//#define MEM_START_ADDRESS 0x08010000
//#define MEM_PAGE32 (32, 0x08010000)


void mem_init(void)
{
	mem_read_config();

	if((Mem.address > 0) && (Mem.address < 10000))
	{
		// ---------- Valid address ----------
		// Nothing to be done

	}
	else
	{
		// ---------- Invalid address ----------
		// Write default values

		//Mem.address = 100;
/*
		Mem.event_ctrl.bit.led_boot_event = _ENABLE;
		Mem.event_ctrl.bit.led_dcc_com = _ENABLE;
		Mem.event_ctrl.bit.led_dcc_com_errors = _ENABLE;
		Mem.event_ctrl.bit.led_mem_write = _ENABLE;
		Mem.event_ctrl.bit.motor_ctrl = CTRL_CLOSE_LOOP;
		Mem.event_ctrl.bit.sleep_enable = _ENABLE;
*/
		//Mem.Ustart_fwd_mV = 6000;
		//Mem.Ustart_bwd_mV = 6000;
		//Mem.Umin_mV = 3000;

		//Mem.motor_p = 8;
		//Mem.motor_i = 2;
		//Mem.motor_d = 0;

		Mem.address = 100;
		Mem.motor_driver.e = DRIVER_UNIVERSAL_MOTOR;
		Mem.motor_ctrl.e = CTRL_CLOSE_LOOP;
		Mem.Rcoil_fwd_mOhm = 18000;

		Mem.Rcoil_bwd_mOhm = 18000;
		Mem.Ustart_fwd_mV = 8000;
		Mem.Ustart_bwd_mV = 8000;
		Mem.Uref_min_start_mV = 450;

		Mem.Uref_min_mV = 300;
		Mem.Uemf_max_stop_mV = 900;
		Mem.Umin_mV = 0;
		Mem.motor_p = 8;

		Mem.motor_i = 2;
		Mem.motor_d = 0;
		Mem.debug_leds = 1;
		Mem.time_after_stop = 0;

		Mem.ratio_spd_fwd_Uref = 300;
		Mem.ratio_spd_bwd_Uref = 300;
		Mem.speed_min_start = 15;
		Mem.speed_min = 10;

		mem_write_config();

		mem_read_config();
	}
}


// Write a flash page (2k Byte)
void mem_write_page(uint32_t page, void *memory, uint32_t memory_size)
{
	uint32_t i;
	uint64_t data;
	FLASH_EraseInitTypeDef erase_config;
	uint32_t page_error;

	// Calculating page address
	uint32_t address = FLASH_BASE + FLASH_PAGE_SIZE*page;

	// Ensure that memory is not bigger than a page (2048 bytes)
	if (memory_size > FLASH_PAGE_SIZE)
	    memory_size = FLASH_PAGE_SIZE;

	erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
	erase_config.Banks = FLASH_BANK_1;
	erase_config.Page = page;
	erase_config.NbPages = 1;

	HAL_FLASH_Unlock();

	// Erase whole page (2 kByte)
	HAL_FLASHEx_Erase(&erase_config, &page_error);

	// Write memory structure into the flash
	for(i=0 ; i < memory_size; i+=8)
	{
		memcpy(&data, ((uint8_t *)memory) + i, 8);

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data);
	}

	HAL_FLASH_Lock();
}

// Read a flash page (2k Byte)
void mem_read_page(uint32_t page, void *memory, uint32_t memory_size)
{
	// Calculating page address
	uint32_t address = FLASH_BASE + FLASH_PAGE_SIZE*page;

	// Ensure that memory is not bigger than a page (2048 bytes)
	if (memory_size > FLASH_PAGE_SIZE)
	    memory_size = FLASH_PAGE_SIZE;

	// Load flash memory into Mem structure
	memcpy(memory, (void *)address, memory_size);
}


void mem_read_config(void)
{
	mem_read_page(PAGE_CONFIG, (void *)&Mem, sizeof(Mem));
}


void mem_write_config(void)
{
	mem_write_page(PAGE_CONFIG, (void *)&Mem, sizeof(Mem));

	mem_read_page(PAGE_CONFIG, (void *)&Mem, sizeof(Mem));
}


void mem_write_motor(void)
{
	mem_read_page(PAGE_MOTOR_STATUS, (void *)&memMotStat, sizeof(memMotStat));

	if(	(memMotStat.page_first_move < PAGE_MOTOR_START) ||
			(memMotStat.page_first_move > PAGE_MOTOR_END) ||
			(memMotStat.page_last_move < PAGE_MOTOR_START) ||
			(memMotStat.page_last_move > PAGE_MOTOR_END))
	{
		memMotStat.page_first_move = PAGE_MOTOR_START;
		memMotStat.page_last_move = PAGE_MOTOR_START;
	}
	else
	{
		memMotStat.page_last_move++;
		if(memMotStat.page_last_move > PAGE_MOTOR_END)
			memMotStat.page_last_move = PAGE_MOTOR_START;

		if(memMotStat.page_last_move == memMotStat.page_first_move)
		{
			memMotStat.page_first_move++;
			if(memMotStat.page_first_move > PAGE_MOTOR_END)
				memMotStat.page_first_move = PAGE_MOTOR_START;
		}
	}

	mem_write_page(memMotStat.page_last_move, (void *)&memMotData, sizeof(memMotData));
	mem_write_page(PAGE_MOTOR_STATUS, (void *)&memMotStat, sizeof(memMotStat));
}


void mem_write_backup_register(backup_idx_t index, uint32_t value)
{
	HAL_PWR_EnableBkUpAccess();

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, MEM_BACKUP_UNVALID_WORD);

	HAL_RTCEx_BKUPWrite(&hrtc, index, value);

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, MEM_BACKUP_MAGIC_WORD);
}

uint32_t mem_read_backup_register(backup_idx_t index)
{
	return HAL_RTCEx_BKUPRead(&hrtc, index);
}

uint32_t mem_is_backup_valid(void)
{
	uint32_t response = FALSE;

	if(mem_read_backup_register(BACKUP_IDX__MAGIC_WORD) == MEM_BACKUP_MAGIC_WORD)
		response = TRUE;

	return response;
}

uint32_t mem_update_backup_crc(void)
{
	uint32_t calculated_crc = 0;

	for(uint32_t i=RTC_BKP_DR0; i<=RTC_BKP_DR30; i++)
	{
		calculated_crc = calculated_crc ^ HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
	}
	return TRUE;
}
