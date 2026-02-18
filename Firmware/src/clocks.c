#include "clocks.h"

void clocks_init(void)
{
    // Update clock frequency to select 24 MHz
    // external crystal and 150 MHz operation
    SET_BIT(RCC->CR, RCC_CR_HSEON);  // Enable high speed external oscillator
    while (READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0);  // Wait for external oscillator to be ready

    // RM0440 p. 278 - The PLL configuration (selection of the input clock and multiplication factor)
    // must be done before enabling the PLL.
    
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);  // Disable the PLL
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 1);  // Wait for the PLL to be fully stopped
    
    // RM0440 p. 291
    // (VCO clock) = f(PLL clock input) × (PLLN / PLLM) = 24 × (75 / 6) = 300 MHz
    // f(PLL_P) = f(VCO clock) / PLLP = 300 MHz / 2 = 150 MHz
    // f(PLL_Q) = f(VCO clock) / PLLQ = 300 MHz / 2 = 150 MHz
    // f(PLL_R) = f(VCO clock) / PLLR = 300 MHz / 2 = 150 MHz
    
    // DS12288 p. 123 - VCO clock must be between 96 MHz and 344 MHz for voltage scaling range 1
    RCC->PLLCFGR = (0b00010 << RCC_PLLCFGR_PLLPDIV_Pos)  // Main division factor for PLL P clock (2)
                 | (1 << RCC_PLLCFGR_PLLREN_Pos)  // Enable PLLR clock output
                 | (0b00 << RCC_PLLCFGR_PLLR_Pos)  // Main division factor for PLL R clock (2)
                 | (1 << RCC_PLLCFGR_PLLQEN_Pos)  // Enable PLLQ clock output
                 | (0b00 << RCC_PLLCFGR_PLLQ_Pos)  // Main division factor for PLL Q clock (2)
                 | (1 << RCC_PLLCFGR_PLLPEN_Pos)  // Enable PLLP clock output
                 | (0 << RCC_PLLCFGR_PLLP_Pos)  // No effect (see PLLPDIV bits above)
                 | (75 << RCC_PLLCFGR_PLLN_Pos)  // Main multiplication factor for the VCO (75)
                 | (0b0101 << RCC_PLLCFGR_PLLM_Pos)  // Main division factor for the PLL input clock (6)
                 | (0b11 << RCC_PLLCFGR_PLLSRC_Pos);  // HSE clock entry
    
    SET_BIT(RCC->CR, RCC_CR_PLLON);  // Enable the PLL
    while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0);  // Wait for the PLL to be locked

    // RM0440 p. 190 - Read access latency
    // For HCLK = 150 MHz (VCORE range 1 normal mode), minimum 4 flash wait states required
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY_Msk, FLASH_ACR_LATENCY_4WS);
    while ((READ_REG(FLASH->ACR) & (FLASH_ACR_LATENCY_Msk)) != FLASH_ACR_LATENCY_4WS);  // Wait for the configuration to take effect
    
    // RM0440 p. 279 - Set clock to intermediate frequency
    // for 1 us min. before transitioning to high speed clock
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE_Msk, 0b1000 << RCC_CFGR_HPRE_Pos);  // Set the AHB prescaler HPRE[3:0] bits to divide the system frequency by 2
    while((READ_REG(RCC->CFGR) & RCC_CFGR_HPRE_Msk) != (0b1000 << RCC_CFGR_HPRE_Pos));  // Wait for the configuration to take effect
    MODIFY_REG(RCC->CFGR, 0b11 << RCC_CFGR_SW_Pos, 0b11 << RCC_CFGR_SW_Pos);  // Switch SYSCLK to PLL
    while((READ_REG(RCC->CFGR) & RCC_CFGR_SWS_Msk) != (0b11 << RCC_CFGR_SWS_Pos));  // Wait for the configuration to take effect
    for (volatile uint32_t i = 0; i < 100; i++)  // Wait at least 1 μs and then reconfigure AHB prescaler
                                                 // bits to the needed HCLK frequency
                                                 // Intermediate frequency is 75 MHz (13.33 ns / operation), if for
                                                 // loop takes 2 operations per loop then 1 us should only be 38 operations
        __NOP();

    MODIFY_REG(RCC->CFGR, 0b1111 << RCC_CFGR_HPRE_Pos, 0b0000 << RCC_CFGR_HPRE_Pos);  // Set the AHB prescaler HPRE[3:0] bits to divide the system frequency by 1
    while((READ_REG(RCC->CFGR) & (0b1111 << RCC_CFGR_HPRE_Pos)) != (0b0000 << RCC_CFGR_HPRE_Pos));  // Wait for the configuration to take effect
    // Note: PPRE1 and PPRE2 bits in RCC_CFGR register are '0' on reset,
    // peripheral clocks APB1 and APB2 are not divided (PCLK1 and PCLK2 = 150 MHz)
    
    SystemCoreClockUpdate();  // Update the SystemCoreClock variable
}