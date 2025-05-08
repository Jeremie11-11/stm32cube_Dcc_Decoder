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
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc1;

ADC_STRUCT Adc;
ADC2_STRUCT Adc2;
ADC_DEBUG_STRUCT AdcDebug;

extern DCC_INSTRUCTION_STRUCT DccInst;

#define TS_CAL1_ADDRESS 0x1FFF75A8
#define TS_CAL1_TEMP (30)
#define TS_CAL2_ADDRESS 0x1FFF75CA
#define TS_CAL2_TEMP (130)

void adc_init(void)
{
	//Adc.val_cnt = ADC_NUMBER_OF_MEASURE;

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

	memcpy(&AdcDebug.ts_cal1, ((void *) TS_CAL1_ADDRESS), 2);
	memcpy(&AdcDebug.ts_cal2, ((void *) TS_CAL2_ADDRESS), 2);

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
	static int32_t val_cnt = ADC_NUMBER_OF_MEASURE;

	static uint32_t Ibridge_sum=0;
	static uint32_t Usupply_sum=0;
	static int32_t temp_sum=0;

	Ibridge_sum += Adc.adc1_meas.data.Ibridge_raw;
	Usupply_sum += Adc.adc1_meas.data.Usupply_raw;


	if(val_cnt <= 0)
	{
		// ---------- Bridge current ----------
		// Calculate current ADC 12 bit with 3.3V Uref
		uint32_t Ibridge_mA = ((Ibridge_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096;
		if(Ibridge_mA < 1)
			Ibridge_mA = 1;

		Adc.Ibridge_mA = Ibridge_mA;


		//Adc.Ibridge_mA = ((Ibridge_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096;
		//if(Adc.Ibridge_mA < 1)
			//Adc.Ibridge_mA = 1;

		// ---------- Power supply voltage ----------

		// Calculate voltage ADC 12 bit with 3.3V Uref with R ratio (R1+R2)/R2 = 9.3
		uint32_t Usupply_mV = ((((Usupply_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096)*93)/10;
		if(Usupply_mV < 100)
			Usupply_mV = 100;

		Adc.Usupply_mV = Usupply_mV;
		Adc.Uin_mV = Usupply_mV;

		/*
		// Calculate voltage ADC 12 bit with 3.3V Uref with R ratio (R1+R2)/R2 = 9.3
		Adc.Uin_mV = ((((Usupply_sum/ADC_NUMBER_OF_MEASURE) * 3300)/4096)*93)/10;
		if(Adc.Uin_mV < 100)
			Adc.Uin_mV = 100;
*/
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
		//Adc.Uin_avg_mV = (Adc.Uin_avg_mV*31 + Adc.Uin_mV)/32;

		// Calculate temperature, Reference manual §16.4.32 Temperature sensor
		//Adc.temp = (((temp_sum/ADC_NUMBER_OF_MEASURE)-AdcDebug.ts_cal1)*(TS_CAL2_TEMP – TS_CAL1_TEMP))/(AdcDebug.ts_cal2-AdcDebug.ts_cal1)+30;
		Adc.temp = (((temp_sum/ADC_NUMBER_OF_MEASURE)-AdcDebug.ts_cal1)*100)/(AdcDebug.ts_cal2-AdcDebug.ts_cal1)+30;

		AdcDebug.val1[AdcDebug.val_idx] = Adc.adc1_meas.data.Ibridge_raw;
		AdcDebug.val2[AdcDebug.val_idx] = Ibridge_sum/ADC_NUMBER_OF_MEASURE;
		AdcDebug.val_idx = (AdcDebug.val_idx+1) & 0x1F;

		val_cnt = ADC_NUMBER_OF_MEASURE;
		Ibridge_sum = 0;
		Usupply_sum = 0;
		temp_sum = 0;

		// Updade PWM (if motor.running is set)
		mot_pwm_update();

	}
	else
	{
		val_cnt--;
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
