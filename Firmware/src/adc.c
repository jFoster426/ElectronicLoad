#include "adc.h"

#include "stm32g474xx.h"

void adc_init(void)
{
    /* -1. Enable clocks for GPIOA and ADC */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;

    /* 0. Configure PA0 as analog (ADC1_IN1) */
    GPIOA->MODER |= (3 << (0 * 2));   // analog mode
    GPIOA->PUPDR &= ~(3 << (0 * 2));  // no pull

    // RM0440 p. 603 - ADC DEEPPWD and ADVREGEN
    // 1. Exit Deep-power-down mode by clearing DEEPPWD bit
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    // 2. Enable the ADC voltage regulator by setting ADVREGEN
    ADC1->CR |= ADC_CR_ADVREGEN;
    // 3. Wait for the startup time to configure the ADC
    for (volatile int i = 0; i < 1000; i++) __NOP();

    // RM0440 p. 605 - Software procedure to calibrate the ADC
    // 1. Ensure DEEPPWD = 0, ADVREGEN = 1 and that ADC voltage regulator startup time has elapsed
    // 2. Ensure that ADEN = 0
    ADC1->CR &= ~ADC_CR_ADEN;
    // 3. Select the input mode for this calibration by setting ADCALDIF = 0 (single-ended input) or ADCALDIF = 1 (differential input)
    // 4. Set ADCAL
    ADC1->CR |= ADC_CR_ADCAL;
    // 5. Wait until ADCAL = 0
    while (ADC1->CR & ADC_CR_ADCAL);
    // 6. The calibration factor can be read from ADC_CALFACT register
    // The calibration data is automatically stored in the ADC registers so it is not required by the user to write it back into the peripheral

    /* 5. Configure ADC */

    /* 12-bit resolution (00) */
    ADC1->CFGR &= ~ADC_CFGR_RES;

    /* Single conversion mode */
    ADC1->CFGR &= ~ADC_CFGR_CONT;

    /* Software trigger */
    ADC1->CFGR &= ~ADC_CFGR_EXTEN;

    /* Sampling time for channel 1 */
    ADC1->SMPR1 |= (4 << ADC_SMPR1_SMP1_Pos);  // ~47.5 cycles

    /* Conversion sequence length = 1 */
    ADC1->SQR1 &= ~ADC_SQR1_L;

    /* First conversion channel = 1 */
    ADC1->SQR1 |= (1 << ADC_SQR1_SQ1_Pos);

    // RM0440 p. 607 - Software procedure to enable the ADC
    // 1. Clear the ADRDY bit in the ADC_ISR register by writing 1
    ADC1->ISR |= ADC_ISR_ADRDY;
    // 2. Set ADEN
    ADC1->CR |= ADC_CR_ADEN;
    // 3. Wait until ADRDY = 1
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
}

uint16_t adc_read(void)
{
    /* Start conversion */
    ADC1->CR |= ADC_CR_ADSTART;

    /* Wait until conversion completes */
    while (!(ADC1->ISR & ADC_ISR_EOC));

    /* Read result */
    return (uint16_t)ADC1->DR;
}