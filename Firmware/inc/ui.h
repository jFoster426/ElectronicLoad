#ifndef __UI_H
#define __UI_H

#include "stm32g4xx.h"

#include "gpio.h"
#include "lptim.h"

const uint8_t BTN_HOLD_THRESH = 15;

volatile uint8_t ui_tmr;
volatile uint8_t ui_btn[8];

volatile uint8_t btn_state[8];
volatile uint8_t btn_state_old[8];
volatile uint8_t btn_hold_cnt[8];


void ui_init(void);

void ui_poll(void);

void EXTI0_IRQHandler(void);

void EXTI9_5_IRQHandler(void);

void EXTI15_10_IRQHandler(void);

#endif