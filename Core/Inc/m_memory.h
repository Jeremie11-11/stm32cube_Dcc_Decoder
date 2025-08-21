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


typedef struct{
	enable_t led_boot_event			: 1;
	enable_t led_dcc_com				: 1;
	enable_t led_dcc_com_errors	: 1;
	enable_t led_mem_write			: 1;
	uint32_t reserved0					: 4+8;

	motor_ctrl_t motor_ctrl			: 1;
	enable_t sleep_enable				: 1;
	uint32_t reserved1					: 6+8;
} bit_events_ctrl_t;


typedef union {
	bit_events_ctrl_t bit;
  uint32_t u32;
} events_ctrl_t;

struct MEM_CONFIG_STRUCT{
	uint32_t address;
	events_ctrl_t event_ctrl;
	uint32_t Ustart_fwd_mV;
	uint32_t Ustart_bwd_mV;

	uint32_t Uref_min_start_mV;
	uint32_t Uref_min_mV;
	uint32_t Uemf_max_stop_mV;
	uint32_t Umin_mV;
	uint32_t motor_p;

	uint32_t motor_i;
	uint32_t motor_d;
	uint32_t pwm_front_light;
	uint32_t pwm_cab_light;

	uint32_t pwm_rear_light;
	uint32_t pwm_opt_light;

	uint32_t pad; // Ensure that MEM_STRUCT is a multiple of 8
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

struct MEM_ASYM_VOLATGE_STRUCT{
	int16_t Uw_mV[256];
	int16_t Ux_mV[256];
	int16_t Uy_mV[256];
	int16_t Uz_mV[256];

	uint32_t index1;
	uint32_t index2;
} __attribute__ ((packed));

void mem_init(void);

void mem_write_page(uint32_t page, void *memory, uint32_t memory_size);

void mem_read_page(uint32_t page, void *memory, uint32_t memory_size);

void mem_read_config(void);

void mem_write_config(void);

void mem_write_motor(void);

void mem_write_asym_data(void);

#endif /* INC_M_MEMORY_H_ */
