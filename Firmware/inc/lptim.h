#ifndef __LPTIM_H
#define __LPTIM_H

#include "stm32g4xx.h"

extern volatile uint8_t buttons_trg_tmr;

void lptim_init(void);

void LPTIM1_IRQHandler(void);

#endif