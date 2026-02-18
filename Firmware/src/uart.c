#include "uart.h"

// UART connections

// TX (out of STM32) PB10
// RX (into STM32)   PE15

void uart_init(uint32_t baud)
{
    // Enable USART3 peripheral clock
    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USART3EN);
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();
    // Refer to RM0440 p. 1698 - Character transmission procedure
    // 1. Program the M bits in USART_CR1 to define the word length
    CLEAR_BIT(USART3->CR1, USART_CR1_M1);
    CLEAR_BIT(USART3->CR1, USART_CR1_M0);
    // 2. Select the desired baud rate using the USART_BRR register
    // RM0440 p. 1707 - USART baud rate generation
    // In case of oversampling by 16, the baud rate is given by the following formula:
    // Tx/Rx baud = usart_ker_ck_pres / USARTDIV
    // Desired baud rate = 115,200
    // usart_ker_ck_pres / 115,200 = USARTDIV
    // 150,000,000 / 115,200 = 1302.08333
    // Actual baud rate = 150,000,000 / 1302 = 115,207.373 (+ 0.0064 %)
    USART3->PRESC = 0;
    USART3->BRR = 150000000UL / baud;
    // 3. Program the number of stop bits in USART_CR2
    MODIFY_REG(USART3->CR2, USART_CR2_STOP_Msk, (0b00 << USART_CR2_STOP_Pos));
    // 4. Enable the USART by writing the UE bit in USART_CR1 register to 1
    SET_BIT(USART3->CR1, USART_CR1_UE);
    // 5. Set the TE bit in USART_CR1 to send an idle frame as first transmission
    SET_BIT(USART3->CR1, USART_CR1_TE);
    
    // Set the pins to the correct state
    // Enable GPIO clocks if not already done
    gpio_init();
    // Configure TX (PB10) to alternate function mode AF7
    gpio_set_alt(GPIOB, 10, 7);
    // Configure RX (PE15) to alternate function mode AF7 
    gpio_set_alt(GPIOE, 15, 7);
}

void uart_putc(uint8_t c)
{
    // Refer to RM0440 p. 1698 - Character transmission procedure
    // 7. Write the data to send in the USART_TDR register
    // Repeat this for each data to be transmitted in case of single buffer
    USART3->TDR = c;
    // FIFO mode is disabled by default
    while (!(USART3->ISR & USART_ISR_TXE));
    // 8. When the last data is written to the USART_TDR register, wait until TC = 1
    // When FIFO mode is disabled, this indicates that the transmission of the last
    // frame is complete
    // This check is required to avoid corrupting the last transmission when the
    // USART is disabled or enters Halt mode
    // Since we are not disabling the UART, this check is not required
}

void uart_puts(uint8_t *s, uint16_t len)
{
    // Transmit characters one at a time
    for (uint32_t i = 0; i < len; i++)
    {
        uart_putc(s[i]);
    }
}