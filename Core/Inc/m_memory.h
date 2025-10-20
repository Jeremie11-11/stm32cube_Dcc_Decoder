/*
 * memory.h
 *
 *  Created on: Apr 3, 2024
 *      Author: J. Frühie
 */

#ifndef INC_M_MEMORY_H_
#define INC_M_MEMORY_H_

#include "main.h"
#include <stdint.h>

// Page 0-63 available, 64 * 2k = 128k flash
#define PAGE_MOTOR_START 32
#define PAGE_MOTOR_END 48
#define PAGE_ASYM_VOLTAGE 49
#define PAGE_MOTOR_STATUS 62
#define PAGE_CONFIG 63

/*
typedef struct{
	//enable_t led_boot_event			: 1;
	//enable_t led_dcc_com				: 1;
	//enable_t led_dcc_com_errors	: 1;
	//enable_t led_mem_write			: 1;
	//uint32_t reserved0					: 4+8;

	motor_ctrl_t motor_ctrl			: 1;
	//enable_t sleep_enable				: 1;
	uint32_t reserved1					: 6+8;
} bit_events_ctrl_t;
*/

typedef union {
	motor_driver_t e;
  uint32_t u32;
} motor_driver_u32_t;

typedef union {
	motor_ctrl_t e;
  uint32_t u32;
} motor_ctrl_u32_t;

struct MEM_CONFIG_STRUCT{
	uint32_t address;
	motor_driver_u32_t motor_driver;
	motor_ctrl_u32_t motor_ctrl;
	uint32_t Rcoil_fwd_mOhm;

	uint32_t Rcoil_bwd_mOhm;
	uint32_t Ustart_fwd_mV;
	uint32_t Ustart_bwd_mV;
	uint32_t Uref_min_start_mV;

	uint32_t Uref_min_mV;
	uint32_t Uemf_max_stop_mV;
	uint32_t Umin_mV;
	uint32_t motor_p;

	uint32_t motor_i;
	uint32_t motor_d;
	uint32_t debug_leds;
	uint32_t pwm_cab_light;

	uint32_t pwm_rear_light;
	uint32_t pwm_opt_light;
	//uint32_t pad; // Ensure that MEM_STRUCT is a multiple of 8
} __attribute__ ((packed));


struct MEM_MOTOR_STATUS_STRUCT{
	uint32_t page_first_move;
	uint32_t page_last_move;
} __attribute__ ((packed));

struct MEM_MOTOR_DATA_STRUCT{
	int16_t Uin_mV[256];
	int16_t Imot_mA[256];
	int16_t Uemf_mV[256];
	int16_t Unew_mV[256];
} __attribute__ ((packed));


void mem_init(void);

void mem_write_page(uint32_t page, void *memory, uint32_t memory_size);

void mem_read_page(uint32_t page, void *memory, uint32_t memory_size);

void mem_read_config(void);

void mem_write_config(void);

void mem_write_motor(void);

// -------------------------------------------------------------------
// ------------------------- Backup register -------------------------
// -------------------------------------------------------------------

#define MEM_BACKUP_MAGIC_WORD 0xA5A5A5A5
#define MEM_BACKUP_UNVALID_WORD 0xFFFFFFFF


typedef enum{
	BACKUP_IDX__MAGIC_WORD = 0,
	BACKUP_IDX__DCC_INFO_0 = 1,
	BACKUP_IDX__DCC_INFO_1 = 2,

} backup_idx_t;


void mem_write_backup_register(backup_idx_t index, uint32_t value);

uint32_t mem_read_backup_register(backup_idx_t index);

uint32_t mem_is_backup_valid(void);

#endif /* INC_M_MEMORY_H_ */
