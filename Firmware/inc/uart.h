#ifndef __UART_H
#define __UART_H

#include "stm32g4xx.h"

#include "gpio.h"

void uart_init(uint32_t baud);

void uart_putc(uint8_t c);

void uart_puts(uint8_t *s, uint16_t len);

#endif