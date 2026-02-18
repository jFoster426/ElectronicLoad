#include "systick.h"

void systick_init(void) {
    extern uint32_t SystemCoreClock;
    uint32_t SystemCoreClockMHz = SystemCoreClock / 1000000UL;
    // SysTick reload register should be loaded to interrupt every 1 ms
    // Value counts down by 1 every processor clock (CLKSOURCE bit is set within SysTick_Config())
    // SysTickTimerPeriod = SystemCoreClockPeriod = (1/SystemCoreClock)
    // SysTickTimerLoadValue = 0.001 / SysTickTimerPeriod
    //                       = 0.001 / (1/SystemCoreClock)
    //                       = 0.001 * SystemCoreClock
    //                       = 1000 * SystemCoreClockMHz
    SysTick_Config(1000 * SystemCoreClockMHz);
    // Reset the SysTick counter used to keep track of ms since start of program execution
    SysTickCounter = 0;
}

void SysTick_Handler(void) {
    // Interrupt routine for the SysTick module
    SysTickCounter++;
}