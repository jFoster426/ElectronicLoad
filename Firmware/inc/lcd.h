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

#define LCD_LINE_LENGTH          20

// Allocate +1 characters in each line so that '\0' is included
extern volatile uint8_t lcd_l1[LCD_LINE_LENGTH+1],
                        lcd_l2[LCD_LINE_LENGTH+1],
                        lcd_l3[LCD_LINE_LENGTH+1],
                        lcd_l4[LCD_LINE_LENGTH+1];

void lcd_init(void);

void lcd_set_data_bus_output(void);

void lcd_set_data_bus_input(void);

void lcd_write_data_bus(uint8_t d);

void lcd_write_rw(uint8_t rw);

void lcd_write_rs(uint8_t rs);

void lcd_write_e(void);

void lcd_write_instruction(uint8_t i);

void lcd_write_data(uint8_t d);

void lcd_write_frame(void);

void lcd_delay(void);

void lcd_enable_backlight(void);

void lcd_disable_backlight(void);

#endif