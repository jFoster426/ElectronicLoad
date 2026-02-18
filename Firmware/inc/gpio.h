#ifndef __GPIO_H
#define __GPIO_H

#include "stm32g4xx.h"

// LED1 on GPIOE1
#define LED1_SET      SET_BIT(GPIOE->BSRR, 1)
#define LED1_CLEAR    SET_BIT(GPIOE->BSRR, (1+16))
#define LED1_WRITE(x) x == 1 ? LED1_SET : LED1_CLEAR

// LED2 on GPIOE0
#define LED2_SET      SET_BIT(GPIOE->BSRR, 0)
#define LED2_CLEAR    SET_BIT(GPIOE->BSRR, (0+16))
#define LED2_WRITE(x) x == 1 ? LED2_SET : LED2_CLEAR

// LED3 on GPIOB9
#define LED3_SET      SET_BIT(GPIOB->BSRR, 9)
#define LED3_CLEAR    SET_BIT(GPIOB->BSRR, (9+16))
#define LED3_WRITE(x) x == 1 ? LED3_SET : LED3_CLEAR

void gpio_init(void);

void gpio_set_output(GPIO_TypeDef * port, uint16_t pin);

void gpio_set_input(GPIO_TypeDef * port, uint16_t pin);

void gpio_set_alt(GPIO_TypeDef * port, uint16_t pin, uint8_t alt_mode);

void gpio_set(GPIO_TypeDef * port, uint16_t pin);

void gpio_reset(GPIO_TypeDef * port, uint16_t pin);

void gpio_write(GPIO_TypeDef * port, uint16_t pin, uint8_t state);

uint8_t gpio_read(GPIO_TypeDef * port, uint16_t pin);

#endif