/*
 * i_gpio.c
 *
 *  Created on: Jun 11, 2025
 *      Author: J. Früh
 */

#include "i_gpio.h"


static void gpio_init_HW_1v2(void);


void gpio_reinit_for_hw_compatibility(void)
{
	gpio_init_HW_1v2();

}


static void gpio_init_HW_1v2(void)
{
#if HARDWARE_VERSION == HARDWARE_VERSION_1v2
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Configure GPIO pin Output Level
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  // Configure GPIO pins as output
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configure GPIOB pins with reset value
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
}

