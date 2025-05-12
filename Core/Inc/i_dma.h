/*
 * dma.h
 *
 *  Created on: 25 mars 2024
 *      Author: J. Früh
 */

#ifndef INC_I_DMA_H_
#define INC_I_DMA_H_

#include "main.h"


#define DMA_GPIO_BUFFER_LENGTH 256
#define DMA_TIME_BUFFER_LENGTH 128
#define DMA_TIME_IDX_MASK (DMA_TIME_BUFFER_LENGTH-1)

#define ASYM_VOLT_TABLE_SIZE 100
#define ASYM_VOLT_STEP_MV (25000/ASYM_VOLT_TABLE_SIZE)

typedef struct{
	uint32_t voltage_0_tab[ASYM_VOLT_TABLE_SIZE];
	uint32_t voltage_1_tab[ASYM_VOLT_TABLE_SIZE];
} ASYM_VOLTAGE_STRUCT;


typedef struct{
	uint8_t gpio_buffer[DMA_GPIO_BUFFER_LENGTH];
	uint32_t val[3];
	uint32_t val_idx;
	uint32_t t_low;
	uint32_t t_high;

	uint32_t idx_in;
	uint32_t idx_out0;
	uint32_t idx_out1;
	uint8_t time_buffer[DMA_TIME_BUFFER_LENGTH];
} DMA_DCC_STRUCT;


void dma_init(void);

void dma2_adc1_callback_full(DMA_HandleTypeDef *hdma);

void dma_callback_halffull(DMA_HandleTypeDef *hdma);

void dma_callback_full(DMA_HandleTypeDef *hdma);

void dma_adc2_callback_halffull(DMA_HandleTypeDef *hdma);

void dma_adc2_callback_full(DMA_HandleTypeDef *hdma);

#endif /* INC_I_DMA_H_ */
