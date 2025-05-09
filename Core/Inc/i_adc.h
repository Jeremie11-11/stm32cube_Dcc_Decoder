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

// Number of conversion in range from ADC1
// corresponding to STM32Cube->ADC1
#define ADC1_DMA_MEASURE_BUFFER_LENGTH 3

#define ADC2_DMA_BUFFER_LENGTH 256

typedef struct{
	uint16_t Ibridge_raw;
	uint16_t Usupply_raw;
	uint16_t Temp_raw;
} ADC1_DMA_STRUCT;

typedef union {
    uint16_t tab[ADC1_DMA_MEASURE_BUFFER_LENGTH];
    ADC1_DMA_STRUCT data;
} ADC1_DMA_UNION;

typedef struct{
	ADC1_DMA_UNION adc1_meas;
	uint16_t asym_volt_tab[ADC2_DMA_BUFFER_LENGTH];
	uint32_t Usupply_mV;	// Power supply voltage
	uint32_t Uin_mV;			// Voltage motor (Usupply - Rds_on * Ibridge)
	uint32_t Ibridge_mA;
	uint32_t Uasym_mV;
	//uint32_t Uin_low_mV;
	//uint32_t Uin_low_avg_mV;
	//uint32_t Udiff_mV;
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

void adc_SetExternalTrigger(ADC_HandleTypeDef *hadc, uint32_t trigger);

void adc1_update_irq(void);

uint16_t adc_get_current(void);

#endif /* INC_I_ADC_H_ */
