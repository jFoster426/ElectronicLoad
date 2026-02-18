#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32g4xx.h"

extern volatile uint32_t SysTickCounter;

void systick_init(void);

void SysTick_Handler(void);

#endif