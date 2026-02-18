#ifndef __DELAY_H
#define __DELAY_H

#include "stm32g4xx.h"

extern volatile uint32_t SysTickCounter;

void delay_ms(uint32_t val);

#endif