#ifndef _UI_H_
#define _UI_H_

#include "stm32g4xx.h"

#include "gpio.h"
#include "lptim.h"
#include "systick.h"

extern volatile const uint8_t  BTN_HOLD_THRESH;
extern volatile const uint32_t BTN_DB_TIME_MS;

extern volatile uint8_t ui_tmr;
extern volatile uint8_t ui_btn[8];

extern volatile uint8_t btn_state[8];
extern volatile uint8_t btn_state_old[8];
extern volatile uint8_t btn_hold_cnt[8];


void ui_init(void);

void ui_poll(void);

void EXTI0_IRQHandler(void);

void EXTI9_5_IRQHandler(void);

void EXTI15_10_IRQHandler(void);

#endif