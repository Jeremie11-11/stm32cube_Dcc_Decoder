/*
 * dma.h
 *
 *  Created on: 25 mars 2024
 *      Author: J. Früh
 */

#ifndef INC_I_DMA_H_
#define INC_I_DMA_H_

#include "main.h"


#define DMA_DCC_BUFFER_LENGTH 256


// Number of conversion in range from ADC1
// corresponding to STM32Cube->ADC1
#define DMA_ADC1_MEASURE_BUFFER_LENGTH 3

typedef struct{
	uint16_t Ibridge_raw;
	uint16_t Usupply_raw;
	uint16_t Temp_raw;
} DMA_ADC1_STRUCT;

typedef union {
    uint16_t tab[DMA_ADC1_MEASURE_BUFFER_LENGTH];
    DMA_ADC1_STRUCT data;
} DMA_ADC1_UNION;

typedef struct{
	uint8_t dcc_gpio_buffer[DMA_DCC_BUFFER_LENGTH];
	uint16_t asym_volt_buffer[DMA_DCC_BUFFER_LENGTH];
	DMA_ADC1_UNION adc1_measure_buffer;
} DMA_STRUCT;


void dma_init(void);

void dma_dcc_callback_halffull(DMA_HandleTypeDef *hdma);

void dma_dcc_callback_full(DMA_HandleTypeDef *hdma);

void dma_asym_callback_halffull(DMA_HandleTypeDef *hdma);

void dma_asym_callback_full(DMA_HandleTypeDef *hdma);

void dma2_adc1_callback_full(DMA_HandleTypeDef *hdma);

#endif /* INC_I_DMA_H_ */
