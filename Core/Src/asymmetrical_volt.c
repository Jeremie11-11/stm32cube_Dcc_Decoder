/*
 * asymmetrical_volt.c
 *
 *  Created on: Jul 22, 2025
 *      Author: J. Früh
 */


#include <asymmetrical_volt.h>
#include "i_dma.h"
#include "i_adc.h"

#if ASYM_DEBUG
#include "dcc_protocol_rx.h"
#include "m_memory.h"
#endif

extern ADC_STRUCT Adc;
extern DMA_STRUCT Dma;

ASYM_VOLTAGE_STRUCT Asym;

#if ASYM_DEBUG
ASYM_DEBUG_STRUCT AsymDebug;
extern DCC_INSTRUCTION_STRUCT DccInst;
extern struct MEM_ASYM_VOLATGE_STRUCT memAsymData;
#endif


void asym_dma_update(uint32_t buffer_full)
{
	uint32_t start_index, stop_index;
	uint32_t Uaysm_mV = 0;
	uint32_t dcc_state = 0;
	static uint32_t dcc_state_old = 2;
	static uint32_t dcc_state_old2 = 2;
	static uint32_t dcc_state_old3 = 2;


	if(buffer_full == FALSE)
	{
		start_index = 0;
		stop_index = DMA_DCC_BUFFER_LENGTH/2;
	}
	else
	{
		start_index = DMA_DCC_BUFFER_LENGTH/2;
		stop_index = DMA_DCC_BUFFER_LENGTH;
	}

	for(uint32_t i=start_index; i<stop_index; i++)
	{
		dcc_state = (Dma.dcc_gpio_buffer[i] >> 6) & 0x01;
		Uaysm_mV = (((Dma.asym_volt_buffer[i] * 3300)/4096)*176)/10;

		uint32_t Uaysm_idx = Uaysm_mV / ASYM_VOLT_STEP_MV;

		if(Uaysm_idx >= ASYM_VOLT_TABLE_SIZE)
			Uaysm_idx = ASYM_VOLT_TABLE_SIZE;

		//if((dcc_state == dcc_state_old) && (dcc_state_old != dcc_state_old2))
		if((dcc_state == dcc_state_old2) && (dcc_state_old2 != dcc_state_old3))
		{
			if(dcc_state == 0)
			{
				if(Uaysm_mV > Asym.Utrack1_mV)
					Asym.Utrack1_mV += 1;
				else
					Asym.Utrack1_mV -= 1;

#if ASYM_DEBUG
				AsymDebug.voltage_1_tab[Uaysm_idx]++;
#endif
			}
			else
			{
				if(Uaysm_mV > Asym.Utrack0_mV)
					Asym.Utrack0_mV += 1;
				else
					Asym.Utrack0_mV -= 1;

#if ASYM_DEBUG
				AsymDebug.voltage_0_tab[Uaysm_idx]++;
#endif
			}
		}

		dcc_state_old3 = dcc_state_old2;
		dcc_state_old2 = dcc_state_old;
		dcc_state_old = dcc_state;

#if ASYM_DEBUG
		memAsymData.Uw_mV[AsymDebug.debug_index] = dcc_state;
		memAsymData.Ux_mV[AsymDebug.debug_index] = Uaysm_idx;
		memAsymData.Uy_mV[AsymDebug.debug_index] = Asym.Utrack0_mV;
		memAsymData.Uz_mV[AsymDebug.debug_index] = Asym.Utrack0_mV - Asym.Utrack1_mV;


		//AsymDebug.debug_tab0[AsymDebug.debug_index] = dcc_state;
		//AsymDebug.debug_tab1[AsymDebug.debug_index] = Uaysm_idx;
		if(AsymDebug.debug_index<255)
			AsymDebug.debug_index++;
		else if(DccInst.dcc_target_speed != 0)
			AsymDebug.debug_index=0;
#endif
	}
	Asym.Uasym_mV = Asym.Utrack0_mV - Asym.Utrack1_mV;
}


void asym_check_for_signal(void)
{
	if(Asym.Uasym_mV > 3000)
	{
		// ---------- RED signal ----------
		DccInst.signal_state = signal_red;

	}
	// Threshold based on measurement: (630mV + 360mV) / 2 = 445mV (2x same state)
	// Threshold based on measurement: (540mV + 140mV) / 2 = 340mV (3x same state) <- More likely to detect the difference
	else if(Asym.Uasym_mV > 340)
	{
		// ---------- ORANGE signal ----------
		DccInst.signal_state = signal_orange;

		GPIO_WRITE(OPT_LIGHT, TRUE);
	}
	else
	{
		// ---------- GREEN signal ----------
		DccInst.signal_state = signal_green;

		GPIO_WRITE(OPT_LIGHT, FALSE);
	}
}

