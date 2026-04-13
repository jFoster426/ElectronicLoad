#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32g4xx.h"

extern volatile uint32_t SysTickCounter;

void delay_ms(uint32_t val);

#endif