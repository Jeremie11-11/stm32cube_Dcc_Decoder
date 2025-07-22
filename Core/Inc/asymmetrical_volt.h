/*
 * asymmetrical_volt.h
 *
 *  Created on: Jul 22, 2025
 *      Author: J. Früh
 */

#ifndef INC_ASYMMETRICAL_VOLT_H_
#define INC_ASYMMETRICAL_VOLT_H_


#include "main.h"

#define ASYM_DEBUG FALSE
#define ASYM_VOLT_TABLE_SIZE 100
#define ASYM_VOLT_STEP_MV (25000/ASYM_VOLT_TABLE_SIZE)


typedef struct{
	uint32_t Utrack0_mV;
	uint32_t Utrack1_mV;
	uint32_t Uasym_mV;
} ASYM_VOLTAGE_STRUCT;

typedef struct{
	uint32_t voltage_0_tab[ASYM_VOLT_TABLE_SIZE];
	uint32_t voltage_1_tab[ASYM_VOLT_TABLE_SIZE];

	uint32_t debug_index;
	uint8_t debug_tab0[200];
	uint8_t debug_tab1[200];
} ASYM_DEBUG_STRUCT;



void asym_dma_update(uint32_t buffer_full);

void asym_check_for_signal(void);

#endif /* INC_ASYMMETRICAL_VOLT_H_ */
