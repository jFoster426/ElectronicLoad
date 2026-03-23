#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "stm32g4xx.h"

volatile uint8_t buttons_trg_tmr;
volatile uint8_t buttons_trg_btn[8];
volatile uint8_t buttons_state[8];

void buttons_init(void);

void buttons_poll(void);

void EXTI0_IRQHandler(void);

void EXTI9_5_IRQHandler(void);

void EXTI15_10_IRQHandler(void);

#endif