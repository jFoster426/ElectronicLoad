#ifndef __LCD_H
#define __LCD_H

#include "stm32g4xx.h"

#include "delay.h"
#include "gpio.h"

// HD44780U datasheet p. 8 - Select read or write mode
#define LCD_RW_MODE_WRITE        0
#define LCD_RW_MODE_READ         1

// HD44780U datasheet p. 8 - Select instruction or data mode
#define LCD_RS_MODE_INSTRUCTION  0
#define LCD_RS_MODE_DATA         1

void lcd_init(void);

void lcd_set_data_bus_output(void);

void lcd_set_data_bus_input(void);

void lcd_write_data_bus(uint8_t d);

void lcd_write_rw(uint8_t rw);

void lcd_write_rs(uint8_t rs);

void lcd_write_e(void);

void lcd_write_instruction(uint8_t i);

void lcd_write_data(uint8_t d);

void lcd_delay(void);

#endif