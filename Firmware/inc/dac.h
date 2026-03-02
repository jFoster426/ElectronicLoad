#ifndef __DAC_H
#define __DAC_H

#include "stm32g4xx.h"

#include "gpio.h"

void dac_init(void);

void dac_write(uint16_t data);

#endif