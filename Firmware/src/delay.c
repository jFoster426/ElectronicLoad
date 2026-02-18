#include <delay.h>

volatile uint32_t SysTickCounter = 0;

void delay_ms(uint32_t val)
{
    uint32_t StartSysTick = SysTickCounter;
    while ((SysTickCounter - StartSysTick) < val);
}