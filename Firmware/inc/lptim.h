#ifndef _LPTIM_H_
#define _LPTIM_H_

#include "stm32g4xx.h"

extern volatile uint8_t ui_tmr;

void lptim_init(void);

void LPTIM1_IRQHandler(void);

#endif