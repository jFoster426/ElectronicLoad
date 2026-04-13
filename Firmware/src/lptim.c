#include "lptim.h"

void lptim_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_LPTIM1EN);
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();

    // Set the clock source to PCLK (150 MHz according to clocks.c)
    CLEAR_BIT(RCC->CCIPR, RCC_CCIPR_LPTIM1SEL_1 | RCC_CCIPR_LPTIM1SEL_0);
    
    // Clock prescaler of 1:128
    SET_BIT(LPTIM1->CFGR, LPTIM_CFGR_PRESC_2 | LPTIM_CFGR_PRESC_1 | LPTIM_CFGR_PRESC_0);

    // Clear the interrupt flag and enable lptim interrupts
    SET_BIT(LPTIM1->ICR, LPTIM_ICR_ARRMCF);
    SET_BIT(LPTIM1->IER, LPTIM_IER_ARRMIE);
    SET_BIT(LPTIM1->CR, LPTIM_CR_ENABLE);
    NVIC_EnableIRQ(LPTIM1_IRQn);

    // RM0440 p. 1551 - The LPTIM_ARR register must only be modified when the LPTIM is enabled
    // Interrupt frequency will be [f(PCLK1) / (PRESC * (ARR + 1))]
    // = [150,000,000 / (128 * (58593 + 1))]
    // = 19.999914667 Hz (interrupt fires 20x/sec.)
    LPTIM1->ARR = 58593;

    // Start lptim in continuous mode
    SET_BIT(LPTIM1->CR, LPTIM_CR_CNTSTRT);
}

void LPTIM1_IRQHandler(void)
{
    if (READ_BIT(LPTIM1->ISR, LPTIM_ISR_ARRM))
    {
        // Clear the interrupt flag
        SET_BIT(LPTIM1->ICR, LPTIM_ICR_ARRMCF);
        
        // Notify the button hold detector that it was called via the timer
        ui_tmr++;
    }
}