#include "dac.h"

void dac_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI4EN);
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();
    // RM0440 p. 1838 - Configuration of SPI
    // 1. Write proper GPIO registers: Configure GPIO for MOSI, MISO and SCK pins
    gpio_set_alt(GPIOE, 2, 5); // DAC.SCLK
    gpio_set_alt(GPIOE, 6, 5); // DAC.SDI
    gpio_set_output(GPIOE, 3); // DAC.CS_N
    SET_BIT(GPIOE->OSPEEDR, 3 << GPIO_OSPEEDR_OSPEED2_Pos); // Set DAC.SCLK to very high speed
    SET_BIT(GPIOE->OSPEEDR, 3 << GPIO_OSPEEDR_OSPEED6_Pos); // Set DAC.SDI to very high speed
    // 2. Write to the SPI_CR1 register:
    //   a. Configure the serial clock baud rate using the BR[2:0] bits
    //      Set to f_PCLK / 256
    MODIFY_REG(SPI4->CR1, SPI_CR1_BR_Msk, 7 << SPI_CR1_BR_Pos);
    //   b. Configure the CPOL and CPHA bits
    //      CPOL reset value is '0', CPHA reset value is '0' -> SPI mode 0
    //   c. Select simplex or half-duplex mode
    //      BIDIMODE reset value is '0' for 2-line unidirectional data mode
    //      RXONLY reset value '0' for full-duplex (even though we don't need receive)
    //   d. Configure the LSBFIRST bit
    //      LSBFIRST reset value is '0' for MSB first
    //   e. Configure the CRCL and CRCEN bits if CRC is needed
    //      CRCEN reset value is '0' for CRC calculation disabled
    //   f. Configure SSM and SSI
    // RM0440 p. 1834 - See Figure 609: When NSS pin is not used in master configuration,
    // it must be internally managed (SSM = 1, SSI = 1) to prevent any MODF error
    SET_BIT(SPI4->CR1, SPI_CR1_SSM);
    SET_BIT(SPI4->CR1, SPI_CR1_SSI);
    //   g. Configure the MSTR bit
    //      MSTR = '1' for master configuration
    MODIFY_REG(SPI4->CR1, SPI_CR1_MSTR_Msk, SPI_CR1_MSTR);
    // 3. Write to the SPI_CR2 register
    //   a. Configure the DS[3:0] bits to select the data length
    //      DS[3..0] = '1111' for 16-bit transfer
    MODIFY_REG(SPI4->CR2, SPI_CR2_DS_Msk, SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0);
    //   b. Configure SSOE
    //      SSOE reset value is '0' for SS output disabled in master mode
    //   c. Set the FRF bit if the TI protocol is required
    //      FRF reset value is '0' for Motorola mode
    //   d. Set the NSSP bit if the NSS pulse mode between two data units is required
    //      NSS reset value is '0' for no NSS pulse
    //   e. Configure the FRXTH bit
    //      This bit is not relevant because we are not receiving any data from the DAC
    //   f. Initialize LDMA_TX and LDMA_RX bits if DMA is used in packed mode
    //      These bits are not relevant because we are not implementing DMA
    // 4. Write to SPI_CRCPR register
    // CRC polynomial is not used
    // 5. Write proper DMA registers: Configure DMA streams dedicated for SPI Tx and Rx in DMA registers if the DMA streams are used
    // DMA is not used
    // Enable the SPI peripheral
    SET_BIT(SPI4->CR1, SPI_CR1_SPE);
}

void dac_write(uint16_t data)
{
    // Ensure any previous transmissions have completed before sending the next one
    while (!READ_BIT(SPI4->SR, SPI_SR_TXE));
    // NSS pin low
    gpio_write(GPIOE, 3, 0);
    // Write to the FIFO to start sending the data out to the DAC over the SPI bus
    SPI4->DR = data;
    // Wait for the transaction to complete before de-asserting NSS pin
    while (SPI4->SR & SPI_SR_BSY);
    // NSS pin high
    gpio_write(GPIOE, 3, 1);
}