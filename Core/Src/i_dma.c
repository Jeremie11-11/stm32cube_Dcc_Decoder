/*
 * dma.c
 *
 *  Created on: 25 mars 2024
 *      Author: J. Früh
 */


#include <i_adc.h>
#include <i_dma.h>


extern DMA_HandleTypeDef hdma_tim15_ch1_up_trig_com;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;


extern ADC_STRUCT Adc;
extern ADC2_STRUCT Adc2;

DMA_DCC_STRUCT Dma;


void dma_init(void)
{
	// ---------- DMA1 ----------
	// Store DCC_SIGNAL for DCC communication
	// Triggered through timer TIM15
	// GPIO peripheral stored into memory(Dma.gpio_buffer)

	// Bit 8 UDE: Update DMA request enable
	TIM15->DIER = TIM_DIER_UDE;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	HAL_DMA_RegisterCallback(&hdma_tim15_ch1_up_trig_com, HAL_DMA_XFER_HALFCPLT_CB_ID, dma_callback_halffull);
	HAL_DMA_RegisterCallback(&hdma_tim15_ch1_up_trig_com, HAL_DMA_XFER_CPLT_CB_ID, dma_callback_full);

	// Enable DMA interrupt
	HAL_DMA_Start_IT(&hdma_tim15_ch1_up_trig_com, (uint32_t) ((&GPIOB->IDR)), (uint32_t) (Dma.gpio_buffer), DMA_GPIO_BUFFER_LENGTH);


	// ---------- DMA1 - channel 2 ----------
	// Store Asymmetrical voltage from ADC2_IN11
	// Triggered through timer TIM15
	// ADC2->DR value stored into memory(Adc2.asym_volt_tab)

	// Enable DMA configuration
	ADC2->CFGR |= ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	HAL_DMA_RegisterCallback(&hdma_adc2, HAL_DMA_XFER_HALFCPLT_CB_ID, dma_adc2_callback_halffull);
	HAL_DMA_RegisterCallback(&hdma_adc2, HAL_DMA_XFER_CPLT_CB_ID, dma_adc2_callback_full);

	// Enable DMA interrupt
	HAL_DMA_Start_IT(&hdma_adc2, (uint32_t) ((&ADC2->DR)), (uint32_t) (Adc.asym_volt_tab), ADC2_DMA_BUFFER_LENGTH);


	// ---------- DMA2 ----------
	// Store ADC measure for bridge_current
	// Triggered through ADC1 EOC(End of conversion)
	// ADC1->DR (measured value) stored into memory(Adc.dma_tab)

	// Enable DMA configuration
	ADC1->CFGR |= ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	HAL_DMA_RegisterCallback(&hdma_adc1, HAL_DMA_XFER_CPLT_CB_ID, dma2_adc1_callback_full);

	HAL_DMA_Start_IT(&hdma_adc1, (uint32_t) ((&ADC1->DR)), (uint32_t) (Adc.adc1_meas.tab), ADC1_DMA_MEASURE_BUFFER_LENGTH);

}


void dma_adc2_callback_halffull(DMA_HandleTypeDef *hdma)
{
  if (hdma == &hdma_adc2)
  {
    //GPIO_WRITE(CAB_LIGHT, TRUE);
    //adc_update();
    //GPIO_WRITE(CAB_LIGHT, FALSE);
  }
}


void dma_adc2_callback_full(DMA_HandleTypeDef *hdma)
{
  if (hdma == &hdma_adc2)
  {
    //GPIO_WRITE(CAB_LIGHT, TRUE);
    //adc_update();
    //GPIO_WRITE(CAB_LIGHT, FALSE);
  }
}


void dma2_adc1_callback_full(DMA_HandleTypeDef *hdma)
{
  if (hdma == &hdma_adc1)
  {
    //GPIO_WRITE(CAB_LIGHT, TRUE);
    adc_update();
    //GPIO_WRITE(CAB_LIGHT, FALSE);
  }
}


void dma_callback_halffull(DMA_HandleTypeDef *hdma)
{
	uint32_t val, i;
	GPIO_TOGGLE(TEST_PIN1);

	for(i=0;i<(DMA_GPIO_BUFFER_LENGTH/2);i++)
	{
		val = (Dma.gpio_buffer[i] >> 6) & 0x01;
		Dma.val[Dma.val_idx++] = val;
		if(Dma.val_idx >= 3)
			Dma.val_idx=0;

		if((Dma.val[0]+Dma.val[1]+Dma.val[2]) >= 2)
		{
			if(Dma.t_low > 0)
			{
				Dma.time_buffer[Dma.idx_in] = Dma.t_low;
				Dma.idx_in = (Dma.idx_in+1) & DMA_TIME_IDX_MASK;
			}
			Dma.t_high++;
			Dma.t_low=0;
		}
		else
		{
			if(Dma.t_high > 0)
			{
				Dma.time_buffer[Dma.idx_in] = Dma.t_high;
				Dma.idx_in = (Dma.idx_in+1) & DMA_TIME_IDX_MASK;
			}
			Dma.t_low++;
			Dma.t_high=0;
		}
	}
}

void dma_callback_full(DMA_HandleTypeDef *hdma)
{
	uint32_t val, i;
	GPIO_TOGGLE(TEST_PIN1);

	for(i=(DMA_GPIO_BUFFER_LENGTH/2);i<DMA_GPIO_BUFFER_LENGTH;i++)
	{
		val = (Dma.gpio_buffer[i] >> 6) & 0x01;
		Dma.val[Dma.val_idx++] = val;
		if(Dma.val_idx >= 3)
			Dma.val_idx=0;

		if((Dma.val[0]+Dma.val[1]+Dma.val[2]) >= 2)
		{
			if(Dma.t_low > 0)
			{
				Dma.time_buffer[Dma.idx_in] = Dma.t_low;
				Dma.idx_in = (Dma.idx_in+1) & DMA_TIME_IDX_MASK;
			}
			Dma.t_high++;
			Dma.t_low=0;
		}
		else
		{
			if(Dma.t_high > 0)
			{
				Dma.time_buffer[Dma.idx_in] = Dma.t_high;
				Dma.idx_in = (Dma.idx_in+1) & DMA_TIME_IDX_MASK;
			}
			Dma.t_low++;
			Dma.t_high=0;
		}
	}
}


