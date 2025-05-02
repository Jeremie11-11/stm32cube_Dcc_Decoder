/*
 * adc.c
 *
 *  Created on: Mar 22, 2024
 *      Author: J. Früh
 */

#include <dcc_protocol_rx.h>
#include <i_adc.h>
#include "motor.h"
#include <string.h>
#include "stm32l4xx_hal_adc.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

ADC_STRUCT Adc;
ADC_DEBUG_STRUCT AdcDebug;

extern DCC_INSTRUCTION_STRUCT DccInst;

#define TS_CAL1_ADDRESS 0x1FFF75A8
#define TS_CAL1_TEMP (30)
#define TS_CAL2_ADDRESS 0x1FFF75CA
#define TS_CAL2_TEMP (130)

void adc_init(void)
{
	Adc.val_cnt = ADC_NUMBER_OF_MEASURE;

	AdcDebug.val_idx = 0;
	AdcDebug.current_idx = 0;

	// ADC calibration
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
	{
		// calibration error
		Error_Handler();
	}

	memcpy(&AdcDebug.ts_cal1, ((void *) TS_CAL1_ADDRESS), 2);
	memcpy(&AdcDebug.ts_cal2, ((void *) TS_CAL2_ADDRESS), 2);

	HAL_ADC_Start(&hadc1);
	//HAL_ADC_Start(&hadc1);
}
/*
void adc_SetExternalTrigger(ADC_HandleTypeDef *hadc, uint32_t trigger)
{
    hadc->Init.ExternalTrigConv = trigger;
    if (HAL_ADC_Init(hadc) != HAL_OK)
    {
        Error_Handler();
    }
}
*/

void adc_update(void)
{
	static uint32_t current_sum=0;
	static uint32_t voltage1_sum=0;
	static uint32_t voltage2_sum=0;
	static int32_t temp_sum=0;
	//uint32_t current_val=0;

//	current_val = Adc.dma_tab[0];

	current_sum += Adc.dma_tab[0];
	voltage1_sum += Adc.dma_tab[1];
	voltage2_sum += Adc.dma_tab[2];
	//temp_sum += Adc.dma_tab[2];

	if(Adc.val_cnt <= 0)
	{
		// Calculate current ADC 12 bit with 3.3V Uref
		Adc.current = ((current_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096;
		if(Adc.current < 1)
			Adc.current = 1;

		// Calculate voltage ADC 12 bit with 3.3V Uref with R ratio (R1+R2)/R2 = 9.3
		Adc.Uin_mV = ((((voltage1_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096)*93)/10;
		if(Adc.Uin_mV < 100)
			Adc.Uin_mV = 100;

		if((Adc.under_voltage == FALSE) && (Adc.Uin_mV < 10000))
		{
			dcc_update_functions();

			Adc.under_voltage = TRUE;
		}
		else if((Adc.under_voltage == TRUE) && (Adc.Uin_mV > 11000))
		{
			dcc_update_functions();

			Adc.under_voltage = FALSE;
		}

		// Uin averaged on 400ms
		Adc.Uin_avg_mV = (Adc.Uin_avg_mV*31 + Adc.Uin_mV)/32;

		// Calculate voltage ADC 12 bit with 3.3V Uref with R ratio (R1+R2)/R2 = 9.3
		Adc.Uin_low_mV = ((((voltage2_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096)*93)/10;
		Adc.Uin_low_avg_mV = (Adc.Uin_low_avg_mV*31 + Adc.Uin_low_mV)/32;

		Adc.Udiff_mV = Adc.Uin_avg_mV - Adc.Uin_low_avg_mV;


		// Calculate temperature, Reference manual §16.4.32 Temperature sensor
		//Adc.temp = (((temp_sum/ADC_NUMBER_OF_MEASURE)-AdcDebug.ts_cal1)*(TS_CAL2_TEMP – TS_CAL1_TEMP))/(AdcDebug.ts_cal2-AdcDebug.ts_cal1)+30;
		Adc.temp = (((temp_sum/ADC_NUMBER_OF_MEASURE)-AdcDebug.ts_cal1)*100)/(AdcDebug.ts_cal2-AdcDebug.ts_cal1)+30;

		AdcDebug.val1[AdcDebug.val_idx] = Adc.dma_tab[0];
		AdcDebug.val2[AdcDebug.val_idx] = current_sum/ADC_NUMBER_OF_MEASURE;
		AdcDebug.val_idx = (AdcDebug.val_idx+1) & 0x1F;

		Adc.val_cnt = ADC_NUMBER_OF_MEASURE;
		current_sum = 0;
		voltage1_sum = 0;
		voltage2_sum = 0;
		temp_sum = 0;

		// Updade PWM (if motor.running is set)
		mot_pwm_update();

	}
	else
	{
		Adc.val_cnt--;
	}
}


uint16_t adc_get_current(void)
{
	//uint32_t i, idx, adc_current=0;
/*
	idx = Adc.val_idx;

	if(idx > 0)
	{
		for(i=0; i<idx; i++)
		{
			adc_current += Adc.val[Adc.val_idx];
		}

		adc_current /= idx;

		// R=1 Ohm, 12 bits ADC, current in mA
		adc_current = (adc_current * 3300)/4096;

		Adc.current_tab[Adc.current_i] = adc_current;
		Adc.current_i = (Adc.current_i + 1) & 0x1F;

		Adc.current = adc_current;

		Adc.val_idx = 0;

		return adc_current;
	}
	else
	{
		return 33333;
	}
	*/
	return 0;
}
