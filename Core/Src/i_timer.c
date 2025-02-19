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

	if(direction == DIR_FORWARDS)
	{
		// Start channel 1
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	}
	else if(direction == DIR_BACKWARDS)
	{
		// Start channel 2
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	}
	else
	{
		// Stop channel 1 and 2
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	}
}

void tim_set_motor_pwm(dir_t direction, uint32_t pwm)
{
	if(direction == DIR_FORWARDS)
	{
		// Start channel 1
		TIM1->CCR1 = pwm;
		TIM1->CCR2 = 0;
	}
	else if(direction == DIR_BACKWARDS)
	{
		// Start channel 2
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
