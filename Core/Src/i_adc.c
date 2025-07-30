/*
 * adc.c
 *
 *  Created on: Mar 22, 2024
 *      Author: J. Früh
 */

#include <dcc_protocol_rx.h>
#include <i_adc.h>
#include <i_dma.h>
#include "motor.h"
#include <string.h>
#include "stm32l4xx_hal_adc.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc1;


extern DMA_STRUCT Dma;


ADC_STRUCT Adc;
ADC_DEBUG_STRUCT AdcDebug;

extern DCC_INSTRUCTION_STRUCT DccInst;

#define TS_CAL1_ADDRESS 0x1FFF75A8
#define TS_CAL1_TEMP (30)
#define TS_CAL2_ADDRESS 0x1FFF75CA
#define TS_CAL2_TEMP (130)

void adc_init(void)
{
	AdcDebug.val_idx = 0;
	AdcDebug.current_idx = 0;

	// ---------- ADC1 ----------
	// ADC used for power supply voltage measurement, motor current measurement

	// ADC calibration
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
	{
		// calibration error
		Error_Handler();
	}

	// Temperature sensor (ts)
	// Datasheet: 				 §3.15.1 Temperature sensor
	// Reference manual: 	§16.4.32 Temperature sensor

	// Load the calibration value for the temperature sensor
	memcpy(&AdcDebug.ts_cal1, ((void *) TS_CAL1_ADDRESS), 2);
	memcpy(&AdcDebug.ts_cal2, ((void *) TS_CAL2_ADDRESS), 2);

	// Correction of the ADC Vref (Using Vref=3.3V vs Vref=3.0V for calibration)
	AdcDebug.ts_cal1 = (AdcDebug.ts_cal1 * 30) / 33;
	AdcDebug.ts_cal2 = (AdcDebug.ts_cal2 * 30) / 33;

	// Calculation of the slope for the temperature sensor (mC° / reg)
	AdcDebug.ts_slope = ((TS_CAL2_TEMP - TS_CAL1_TEMP)*1000) / (AdcDebug.ts_cal2-AdcDebug.ts_cal1);

	HAL_ADC_Start(&hadc1);

	// ---------- ADC2 ----------
	// ADC used for asymmetrical voltage measurement

	// ADC calibration
	if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
	{
		// calibration error
		Error_Handler();
	}

	HAL_ADC_Start(&hadc2);
}


void adc_measure_update(void)
{
	static int32_t val_cnt = ADC_NBR_MEASURE_FOR_AVERAGE;

	static uint32_t Ibridge_sum=0;
	static uint32_t Usupply_sum=0;
	static int32_t Ctemp_sum=0;

	Ibridge_sum += Dma.adc1_measure_buffer.data.Ibridge_raw;
	Usupply_sum += Dma.adc1_measure_buffer.data.Usupply_raw;
	Ctemp_sum += Dma.adc1_measure_buffer.data.Temp_raw;

	if(val_cnt <= 0)
	{
		// ---------- Bridge current ----------
		// Calculate current with 12 bit ADC, 3.3V Uref and 1.0 Ohm Rshunt
		int32_t Ibridge_mA = ((((Ibridge_sum/ADC_NBR_MEASURE_FOR_AVERAGE) * 3300)/4096) * 100)/100;
		if(Ibridge_mA < 1)
			Ibridge_mA = 1;

		Adc.Ibridge_mA = Ibridge_mA;

		// ---------- Power supply voltage ----------
		// Calculate voltage ADC 12 bit with 3.3V Uref with R ratio (R1+R2)/R2 = 9.3
		int32_t Usupply_mV = ((((Usupply_sum/ADC_NBR_MEASURE_FOR_AVERAGE) * 3300)/4096)*93)/10;
		if(Usupply_mV < 100)
			Usupply_mV = 100;

		Adc.Usupply_mV = Usupply_mV;
		Adc.Uin_mV = Usupply_mV;

		// ---------- Chip temperature ----------
		// Calculate temperature using calibration values
		Adc.Temp_C = (((Ctemp_sum/ADC_NBR_MEASURE_FOR_AVERAGE) - AdcDebug.ts_cal1) * AdcDebug.ts_slope + 30000)/1000;


		AdcDebug.val1[AdcDebug.val_idx] = Dma.adc1_measure_buffer.data.Ibridge_raw;
		AdcDebug.val2[AdcDebug.val_idx] = Ibridge_sum/ADC_NBR_MEASURE_FOR_AVERAGE;
		AdcDebug.val_idx = (AdcDebug.val_idx+1) & 0x1F;

		val_cnt = ADC_NBR_MEASURE_FOR_AVERAGE;
		Ibridge_sum = 0;
		Usupply_sum = 0;
		Ctemp_sum = 0;

		// Updade PWM (if motor.running is set)
		mot_pwm_update();

	}
	else
	{
		val_cnt--;
	}
}


