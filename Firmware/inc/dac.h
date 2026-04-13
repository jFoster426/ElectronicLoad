#ifndef _DAC_H_
#define _DAC_H_

#include "stm32g4xx.h"

#include "gpio.h"

void dac_init(void);

void dac_write(uint16_t data);

#endif