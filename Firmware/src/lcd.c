#include "lcd.h"

// LCD connections

// RS   PB11
// RW   PB12
// E    PB13
// BKLT PB14
// OE   PB15

// DB0  PD15
// DB1  PD14
// DB2  PD13
// DB3  PD12
// DB4  PD11
// DB5  PD10
// DB6  PD9
// DB7  PD8

void lcd_init(void)
{
    gpio_init();

    gpio_set_output(GPIOB, 15);
    gpio_set(GPIOB, 15);  // LCD.OE high, enable buffers

    gpio_set_output(GPIOB, 11);
    gpio_set_output(GPIOB, 12);
    gpio_set_output(GPIOB, 13);
    gpio_set_output(GPIOB, 14);
    gpio_set_output(GPIOB, 15);

    delay_ms(50);

    lcd_write_instruction(0b00110000);
    delay_ms(15);
    lcd_write_instruction(0b00110000);
    delay_ms(5);
    lcd_write_instruction(0b00110000);
    delay_ms(1);
    lcd_write_instruction(0b00111000);
    delay_ms(8);
    lcd_write_instruction(0x08); // display OFF
    delay_ms(8);
    lcd_write_instruction(0b00000001);
    delay_ms(8);
    lcd_write_instruction(0b00000110);
    delay_ms(8);
    lcd_write_instruction(0x0C); // display ON
}

void lcd_set_data_bus_output(void)
{
    gpio_set_output(GPIOD, 8);
    gpio_set_output(GPIOD, 9);
    gpio_set_output(GPIOD, 10);
    gpio_set_output(GPIOD, 11);
    gpio_set_output(GPIOD, 12);
    gpio_set_output(GPIOD, 13);
    gpio_set_output(GPIOD, 14);
    gpio_set_output(GPIOD, 15);
}

void lcd_set_data_bus_input(void)
{
    gpio_set_input(GPIOD, 8);
    gpio_set_input(GPIOD, 9);
    gpio_set_input(GPIOD, 10);
    gpio_set_input(GPIOD, 11);
    gpio_set_input(GPIOD, 12);
    gpio_set_input(GPIOD, 13);
    gpio_set_input(GPIOD, 14);
    gpio_set_input(GPIOD, 15);
}

void lcd_write_data_bus(uint8_t d)
{
    gpio_write(GPIOD, 15, (d & (1 << 0)) ? 1 : 0);
    gpio_write(GPIOD, 14, (d & (1 << 1)) ? 1 : 0);
    gpio_write(GPIOD, 13, (d & (1 << 2)) ? 1 : 0);
    gpio_write(GPIOD, 12, (d & (1 << 3)) ? 1 : 0);
    gpio_write(GPIOD, 11, (d & (1 << 4)) ? 1 : 0);
    gpio_write(GPIOD, 10, (d & (1 << 5)) ? 1 : 0);
    gpio_write(GPIOD,  9, (d & (1 << 6)) ? 1 : 0);
    gpio_write(GPIOD,  8, (d & (1 << 7)) ? 1 : 0);
}

void lcd_write_rw(uint8_t rw)
{
    if (rw == LCD_RW_MODE_WRITE) lcd_set_data_bus_output();
    if (rw == LCD_RW_MODE_READ)  lcd_set_data_bus_input();
    gpio_write(GPIOB, 12, rw);
}

void lcd_write_rs(uint8_t rs)
{
    gpio_write(GPIOB, 11, rs);
}

void lcd_write_e(void)
{
    lcd_delay();
    gpio_set(GPIOB, 13);
    for (volatile uint32_t i = 0; i < 100; i++) __NOP();
    gpio_reset(GPIOB, 13);
}

void lcd_write_instruction(uint8_t i)
{
    lcd_write_rw(LCD_RW_MODE_WRITE);
    lcd_write_rs(LCD_RS_MODE_INSTRUCTION);
    lcd_write_data_bus(i);
    lcd_write_e();
}

void lcd_write_data(uint8_t d)
{
    lcd_write_rw(LCD_RW_MODE_WRITE);
    lcd_write_rs(LCD_RS_MODE_DATA);
    lcd_write_data_bus(d);
    lcd_write_e();
}

void lcd_delay(void)
{
    // TODO: Implement some kind of LCD delay that changes
    // how quickly data is sent to the LCD
    delay_ms(1);
}