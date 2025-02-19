/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include <m_counter.h>
#include <m_debug.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

#define FALSE 0U
#define TRUE 1U
#define DISABLE 0U
#define ENABLE 1U


typedef enum{
	_DISABLE = 0,
	_ENABLE = 1
} enable_t;

typedef enum{
	DIR_FORWARDS = 0,
	DIR_BACKWARDS = 1,
	DIR_STOPPED = 2
}dir_t;

typedef enum{
	CTRL_OPEN_LOOP = 0,
	CTRL_CLOSE_LOOP = 1
}motor_ctrl_t;


typedef enum{
	front_light 	= 1 << 0,
	cab_light 		= 1 << 1,
	rear_light 		= 1 << 2,
	opt_light			= 1 << 3
}light_t;

struct RegPin {
	GPIO_TypeDef* reg;
	uint16_t pin;
};

#define LED1_GREEN {GPIOB, GPIO_PIN_14}
#define LED2_RED {GPIOB, GPIO_PIN_15}
#define TEST_PIN1 {GPIOB, GPIO_PIN_10}
#define TEST_PIN2 {GPIOA, GPIO_PIN_3}

#define FRONT_LIGHT {GPIOB, GPIO_PIN_13}
#define CAB_LIGHT {GPIOB, GPIO_PIN_11}
#define REAR_LIGHT {GPIOB, GPIO_PIN_12}
#define OPT_LIGHT {GPIOB, GPIO_PIN_10}

#define PIN_CURRENT_EN {GPIOC, GPIO_PIN_13}	// PIN to enable higher input current

// GPIO_WRITE(GREEN_LED, TRUE);
#define GPIO_WRITE(signal, state) \
    do { \
        struct RegPin s = signal; \
        if (state) { \
            s.reg->BSRR = (uint32_t)(s.pin);  /* Set pin to 1 */ \
        } else { \
            s.reg->BSRR = (uint32_t)(s.pin << 16); /* Set pin to 0 */ \
        } \
    } while(0)

// GPIO_READ(GREEN_LED);
#define GPIO_READ(signal) \
    ({ \
        struct RegPin s = signal; \
        (s.reg->IDR & s.pin) ? 1 : 0;  /* Return pin state (1 or 0) */ \
    })

// GPIO_TOGGLE(GREEN_LED);
#define GPIO_TOGGLE(signal) \
    do { \
        struct RegPin s = signal; \
        s.reg->ODR ^= s.pin;  /* Inverse pin state */ \
    } while(0)

#define GPIO_CONFIG_OUTPUT(signal) \
    do { \
        struct RegPin s = signal; \
        uint32_t pin_number = __builtin_ctz(s.pin); \
        uint32_t temp = s.reg->MODER; \
        temp &= ~(GPIO_MODER_MODE0_Msk << (pin_number * 2));  /* Effacer le mode actuel de la pin */ \
        temp |= (GPIO_MODE_OUTPUT_PP << (pin_number * 2));    /* Configurer en mode sortie push-pull */ \
        s.reg->MODER = temp;                                  /* Écrire la configuration dans le registre MODER */ \
        s.reg->OTYPER &= ~(GPIO_OTYPER_OT0 << pin_number);    /* Mettre à 0 le bit dans OTYPER (Push-Pull) */ \
    } while(0)

#define GPIO_CONFIG_INPUT(signal) \
    do { \
        struct RegPin s = signal; \
        uint32_t pin_number = __builtin_ctz(s.pin); \
        uint32_t temp = s.reg->MODER; \
        temp &= ~(GPIO_MODER_MODE0_Msk << (pin_number * 2));  /* Effacer le mode actuel de la pin (00 = Entrée) */ \
        s.reg->MODER = temp;                                  /* Écrire la configuration dans le registre MODER */ \
        temp = s.reg->PUPDR; \
        temp &= ~(GPIO_PUPDR_PUPD0_Msk << (pin_number * 2));  /* Effacer pull-up et pull-down (00 = Pas de pull) */ \
        s.reg->PUPDR = temp;                                  /* Écrire la configuration dans le registre PUPDR */ \
    } while(0)

#define GPIO_CONFIG_OPEN_DRAIN(signal) \
    do { \
        struct RegPin s = signal; \
        uint32_t pin_number = __builtin_ctz(s.pin); \
        uint32_t temp = s.reg->MODER; \
        temp &= ~(GPIO_MODER_MODE0_Msk << (pin_number * 2)); \
        temp |= (GPIO_MODE_OUTPUT_OD << (pin_number * 2)); \
        s.reg->MODER = temp; \
        temp = s.reg->OTYPER; \
        temp |= (GPIO_OTYPER_OT0 << pin_number); \
        s.reg->OTYPER = temp; \
    } while(0)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
