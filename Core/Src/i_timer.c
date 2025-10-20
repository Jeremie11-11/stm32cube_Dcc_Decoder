/*
 * timer.c
 *
 *  Created on: Feb 9, 2023
 *      Author: J. Frueh
 */

#include <i_adc.h>
#include <i_timer.h>
#include <m_memory.h>


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim16;

//extern ADC_HandleTypeDef hadc1;
extern struct MEM_CONFIG_STRUCT Mem;


void tim_init(void)
{
	// ---------- TIM1 ----------
	// Timer used for motor PWM generation and to trigger ADC sequence
	tim_set_motor_bridge(DIR_STOPPED);

	// ---------- TIM2 ----------
	// Timer used for lights PWM generation (Dimmer)
	TIM2->ARR = PWM_LIGHT_PERIOD_CNT;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

	// ---------- TIM15 ----------
	// Timer used to trigger DMA request on GPIO (DCC decoder)
	HAL_TIM_Base_Start(&htim15);

	// ---------- TIM16 ----------
	//HAL_TIM_Base_Start_IT(&htim16);
}


void tim_set_motor_bridge(dir_t direction)
{
	TIM1->ARR = PWM_MOTOR_PERIOD_CNT;
	TIM1->CCR3 = 1;

	//tim_set_tim1_channel_polarity();

	if((direction != DIR_FORWARDS) && (direction != DIR_BACKWARDS))
	{
		// ----- Stop channel 1 and 2 -----

		GPIO_WRITE(CH1_HIGH_SIDE, FALSE);
		GPIO_WRITE(CH2_HIGH_SIDE, FALSE);

		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);


		/*
		__HAL_TIM_DISABLE(&htim1);
		__HAL_TIM_SET_COUNTER(&htim1, 0);

		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		htim1.Instance->EGR = TIM_EGR_UG; // Force update, load preloads

		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

		__HAL_TIM_ENABLE(&htim1);
*/
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	}
	else if(Mem.motor_driver.e == DRIVER_UNIVERSAL_MOTOR)
	{
#if (HARDWARE_VERSION == HARDWARE_VERSION_1v1) || (HARDWARE_VERSION == HARDWARE_VERSION_1v2)
		if(direction == DIR_FORWARDS)
		{
			// Start channel 1
			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
		else
		{
			// Start channel 2
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
			HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
#else
		if(direction == DIR_FORWARDS)
		{
			// ----- Universal motor forwards -----
			// H-bridge high side disabled
			// Using channel TIM1_CH1N with PWM drive
			// TIM1_CH2N set to zero

			// Disable H-bridge high side
			GPIO_WRITE(CH1_HIGH_SIDE, FALSE);
			GPIO_WRITE(CH2_HIGH_SIDE, FALSE);

			// Init H-bridge low side
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
		else
		{
			// ----- Universal motor backwards -----
			// H-bridge high side disabled
			// Using channel TIM1_CH2N with PWM drive
			// TIM1_CH1N set to zero

			// Disable H-bridge high side
			GPIO_WRITE(CH1_HIGH_SIDE, FALSE);
			GPIO_WRITE(CH2_HIGH_SIDE, FALSE);

			// Init H-bridge low side
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
#endif

	}
	else if(Mem.motor_driver.e == DRIVER_DC_MOTOR)
	{
		if(direction == DIR_FORWARDS)
		{
			// ----- DC motor forwards -----
			// H-bridge high side enabled (V+ on CH2)
			// Using channel TIM1_CH1N with PWM drive
			// TIM1_CH2N set to zero

			// Init H-bridge low side
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

			// Enable H-bridge high side
			GPIO_WRITE(CH1_HIGH_SIDE, FALSE);
			GPIO_WRITE(CH2_HIGH_SIDE, TRUE);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
		else
		{
			// ----- DC motor backwards -----
			// H-bridge high side enabled (V+ on CH1)
			// Using channel TIM1_CH2N with PWM drive
			// TIM1_CH1N set to zero

			// Init H-bridge low side
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
			HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

			// Enable H-bridge high side
			GPIO_WRITE(CH1_HIGH_SIDE, TRUE);
			GPIO_WRITE(CH2_HIGH_SIDE, FALSE);

			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		}
	}

}
/*
void tim_set_tim1_channel_polarity()
{
  TIM_OC_InitTypeDef sConfigOC = {0};

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;

  if(Mem.motor_driver.e == DRIVER_UNIVERSAL_MOTOR)
  {
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  }
  else if(Mem.motor_driver.e == DRIVER_DC_MOTOR)
  {
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		//sConfigOC.OCNPolarity = TIM_OCNPOLARITY_LOW;
		sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  }

	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_Handler();
	}
}
*/
void tim_set_motor_pwm(dir_t direction, uint32_t pwm)
{
	if(direction == DIR_FORWARDS)
	{
		// Start channel 1
		TIM1->CCR1 = pwm;
		//TIM1->CCR2 = PWM_MOTOR_PERIOD_CNT;
		TIM1->CCR2 = 0;
	}
	else if(direction == DIR_BACKWARDS)
	{
		// Start channel 2
		//TIM1->CCR1 = PWM_MOTOR_PERIOD_CNT;
		TIM1->CCR1 = 0;
		TIM1->CCR2 = pwm;
	}
	else
	{
		// Stop channel 1 and 2
		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
	}
}

/*
void tim_set_light(light_t light, uint8_t function)
{
	uint32_t state = function & light;

	if(light == front_light)
	{
		if(state == DISABLE)
			TIM2->CCR1 = 0;
		else
			TIM2->CCR1 = Mem.pwm_front_light;
	}
	else if(light == cab_light)
	{
		if(state == DISABLE)
			TIM2->CCR2 = 0;
		else
			TIM2->CCR2 = Mem.pwm_cab_light;
	}
	else if(light == rear_light)
	{
		if(state == DISABLE)
			TIM2->CCR3 = 0;
		else
			TIM2->CCR3 = Mem.pwm_rear_light;
	}
	else if(light == opt_light)
	{
		if(state == DISABLE)
			TIM2->CCR4 = 0;
		else
			TIM2->CCR4 = Mem.pwm_opt_light;
	}
}
*/

void tim_BusyWait_us(uint32_t us)
{
    uint32_t    nbr_10us;   // Number of ticks to wait
    //bool wait = true;

    nbr_10us = us / 10;
    if(nbr_10us <= 0)
    	nbr_10us = 1;

    TIM15->CNT = 0;

    HAL_TIMEx_OCN_Start(&htim15, TIM_CHANNEL_1);

    while(TIM15->CNT < nbr_10us) {} // Wait

    HAL_TIMEx_OCN_Stop(&htim15, TIM_CHANNEL_1);
}


void tim_test_timer(void)
{
	uint32_t i;

	for(i=0;i<10;i++)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
		tim_BusyWait_us(1000);
	}
}
