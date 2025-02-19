/*
 * adc.h
 *
 *  Created on: Mar 22, 2024
 *      Author: J. Früh
 */

#ifndef INC_I_ADC_H_
#define INC_I_ADC_H_

#include "main.h"

// Has to be a power of 2
#define ADC_NUMBER_OF_MEASURE 512

// Has to be a multiple of 3
#define DMA_ADC_BUFFER_LENGTH 3

typedef struct{
	uint16_t dma_tab[DMA_ADC_BUFFER_LENGTH];
	int32_t val_cnt;
	uint32_t current;
	uint32_t Uin_mV;
	uint32_t Uin_avg_mV;
	int32_t temp;
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

void adc_update(void);

uint16_t adc_get_current(void);

#endif /* INC_I_ADC_H_ */
