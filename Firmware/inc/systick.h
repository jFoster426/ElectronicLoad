#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "stm32g4xx.h"

extern volatile uint32_t SysTickCounter;

void systick_init(void);

void SysTick_Handler(void);

#endif