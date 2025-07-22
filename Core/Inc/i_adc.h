/*
 * adc.h
 *
 *  Created on: Mar 22, 2024
 *      Author: J. Früh
 */

#ifndef INC_I_ADC_H_
#define INC_I_ADC_H_

#include "main.h"

//
#define ADC_NBR_MEASURE_FOR_AVERAGE 512

typedef struct{

	uint32_t Usupply_mV;	// Power supply voltage
	uint32_t Uin_mV;			// Voltage motor (Usupply - Rds_on * Ibridge)
	uint32_t Ibridge_mA;

	int32_t Temp_dC;
	uint32_t under_voltage: 1;
} ADC_STRUCT;



typedef struct{
	uint16_t ts_cal1;
	uint16_t ts_cal2;
	uint16_t val1[32];
	uint16_t val2[32];
	uint32_t val_idx;
	uint16_t current_tab[32];
	uint32_t current_idx;
} ADC_DEBUG_STRUCT;

void adc_init(void);

void adc_measure_update(void);

#endif /* INC_I_ADC_H_ */
