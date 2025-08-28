/*
 * i_gpio.h
 *
 *  Created on: Jun 11, 2025
 *      Author: J. Früh
 */

#ifndef INC_I_GPIO_H_
#define INC_I_GPIO_H_

#include "main.h"


typedef enum{
    LIGHT_ALL_OFF	= 0,
		LIGHT_ALL_ON,
		LIGHT_RED,
		LIGHT_ORANGE,
		LIGHT_GREEN,
		LIGHT_BLUE_ON,
		LIGHT_BLUE_OFF
} light_color_t;




// --------------------------------------------------
// -------------- GPIO pin definition ---------------
// --------------------------------------------------

#define LED1_GREEN {GPIOB, GPIO_PIN_12}
#define LED2_RED {GPIOB, GPIO_PIN_15}
#define TEST_PIN1 {GPIOB, GPIO_PIN_10}
#define TEST_PIN2 {GPIOB, GPIO_PIN_1}

#define FRONT_LIGHT {GPIOC, GPIO_PIN_14}
#define CAB_LIGHT {GPIOC, GPIO_PIN_15}
#define REAR_LIGHT {GPIOH, GPIO_PIN_0}
#define OPT_LIGHT {GPIOA, GPIO_PIN_2}

#define DEBUG_LED_ORANGE {GPIOA, GPIO_PIN_12}
#define DEBUG_LED_RED {GPIOA, GPIO_PIN_10}

#define PIN_CURRENT_EN {GPIOC, GPIO_PIN_13}	// PIN to enable higher input current

// --------------------------------------------------
// --------------- GPIO function macro --------------
// --------------------------------------------------

struct RegPin {
	GPIO_TypeDef* reg;
	uint16_t pin;
};

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

// --------------------------------------------------
// --------------- GPIO function --------------
// --------------------------------------------------

void gpio_reinit(void);


#endif /* INC_I_GPIO_H_ */
