#include <stdlib.h>
#include <stdio.h>

#include "stm32g4xx.h"

#include "clocks.h"
#include "delay.h"
#include "gpio.h"
#include "lcd.h"
#include "systick.h"
#include "uart.h"

int main(void)
{
    clocks_init();
    systick_init();
    gpio_init();
    gpio_set_output(GPIOE, 1);  // LED1 (red)
    uart_init(9600);
    lcd_init();

    gpio_set_output(GPIOD, 4);  // LATCH
    gpio_set(GPIOD, 4);         // Enable power latch

    // gpio_set_output(GPIOB, 7);  // GPWR_EN
    // gpio_set(GPIOB, 7);         // Enable +15V/-5V gate power supplies

    lcd_init();
    lcd_write_data('H');
    lcd_write_data('e');
    lcd_write_data('l');
    lcd_write_data('l');
    lcd_write_data('o');
    lcd_write_data(',');
    lcd_write_data(' ');
    lcd_write_data('W');
    lcd_write_data('o');
    lcd_write_data('r');
    lcd_write_data('l');
    lcd_write_data('d');
    lcd_write_data('!');
    

    
    while (1) {
        printf("Hello, World!\n");

        gpio_set(GPIOE, 1);
        delay_ms(1000);
        gpio_reset(GPIOE, 1);
        delay_ms(1000);
    }
    return 0;
}
