#ifndef __ADC_H
#define __ADC_H

#include "stm32g4xx.h"

#include "gpio.h"

#define ADC1_NUM_CHANNELS 8

extern volatile uint16_t adc1_dma_buffer[ADC1_NUM_CHANNELS];

void adc_init(void);

#endif