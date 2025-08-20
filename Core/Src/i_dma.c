/*
 * dma.c
 *
 *  Created on: 25 mars 2024
 *      Author: J. Früh
 */


#include <i_dma.h>
#include <i_adc.h>
#include <dcc_physical_layer.h>


extern DMA_HandleTypeDef hdma_tim15_ch1_up_trig_com;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;


DMA_STRUCT Dma;


void dma_init(void)
{
	// ---------- DMA1 ----------
	// Store DCC_SIGNAL for DCC communication
	// Triggered through timer TIM15
	// GPIO peripheral stored into memory(Dma.dcc_gpio_buffer)

	// Bit 8 UDE: Update DMA request enable
	TIM15->DIER = TIM_DIER_UDE;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	HAL_DMA_RegisterCallback(&hdma_tim15_ch1_up_trig_com, HAL_DMA_XFER_HALFCPLT_CB_ID, dma_dcc_callback_halffull);
	HAL_DMA_RegisterCallback(&hdma_tim15_ch1_up_trig_com, HAL_DMA_XFER_CPLT_CB_ID, dma_dcc_callback_full);

	// Enable DMA interrupt
	HAL_DMA_Start_IT(&hdma_tim15_ch1_up_trig_com, (uint32_t) ((&GPIOB->IDR)), (uint32_t) (Dma.dcc_gpio_buffer), DMA_DCC_BUFFER_LENGTH);


	// ---------- DMA1 - channel 2 ----------
	// Store Asymmetrical voltage from ADC2_IN11
	// Triggered through timer TIM15
	// ADC2->DR value stored into memory(Dma.asym_volt_buffer)

	// Enable DMA configuration
	//ADC2->CFGR |= ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	//HAL_DMA_RegisterCallback(&hdma_adc2, HAL_DMA_XFER_HALFCPLT_CB_ID, dma_asym_callback_halffull);
	//HAL_DMA_RegisterCallback(&hdma_adc2, HAL_DMA_XFER_CPLT_CB_ID, dma_asym_callback_full);

	// Enable DMA interrupt
	//HAL_DMA_Start_IT(&hdma_adc2, (uint32_t) ((&ADC2->DR)), (uint32_t) (Dma.asym_volt_buffer), DMA_DCC_BUFFER_LENGTH);


	// ---------- DMA2 ----------
	// Store ADC measure for bridge_current
	// Triggered through ADC1 EOC(End of conversion)
	// ADC1->DR (measured value) stored into memory(Dma.adc1_measure_buffer.tab)

	// Enable DMA configuration
	ADC1->CFGR |= ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

	/*
	 HAL_DMA_XFER_CPLT_CB_ID          = 0x00U,    // Full transfer
	 HAL_DMA_XFER_HALFCPLT_CB_ID      = 0x01U,    // Half transfer
	*/
	HAL_DMA_RegisterCallback(&hdma_adc1, HAL_DMA_XFER_CPLT_CB_ID, dma2_adc1_callback_full);

	HAL_DMA_Start_IT(&hdma_adc1, (uint32_t) ((&ADC1->DR)), (uint32_t) (Dma.adc1_measure_buffer.tab), DMA_ADC1_MEASURE_BUFFER_LENGTH);

}


void dma_dcc_callback_halffull(DMA_HandleTypeDef *hdma)
{
  if(hdma == &hdma_tim15_ch1_up_trig_com)
  {
  	dcc_dma_update(FALSE);
  }
}


void dma_dcc_callback_full(DMA_HandleTypeDef *hdma)
{
  if(hdma == &hdma_tim15_ch1_up_trig_com)
  {
  	dcc_dma_update(TRUE);
  }
}


void dma_asym_callback_halffull(DMA_HandleTypeDef *hdma)
{
  if(hdma == &hdma_adc2)
  {
  	//asym_dma_update(FALSE);
  }
}


void dma_asym_callback_full(DMA_HandleTypeDef *hdma)
{
  if(hdma == &hdma_adc2)
  {
  	//asym_dma_update(TRUE);
  }
}


void dma2_adc1_callback_full(DMA_HandleTypeDef *hdma)
{
  if(hdma == &hdma_adc1)
  {
  	adc_measure_update();
  }
}





