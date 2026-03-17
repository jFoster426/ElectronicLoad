#include "adc.h"

// ADC channel mappings (ADC1)
// I_OUT(CL1)      ADC.CH1     ADC1_IN1    (PA0)
// I_OUT(CL3)      ADC.CH3     ADC1_IN8    (PC2)
// I_OUT(CL4)      ADC.CH4     ADC1_IN4    (PA3)
// I_OUT(CL5)      ADC.CH5     ADC1_IN6    (PA4)
// I_OUT(CL6)      ADC.CH6     ADC1_IN2    (PA1)
// I_OUT(CL7)      ADC.CH7     ADC1_IN7    (PC1)
// I_OUT(CL8)      ADC.CH8     ADC1_IN9    (PC3)
// LV_AMP          ADC.CH9     ADC1_IN3    (PA2)

// ADC channel mappings (ADC2)
// I_OUT(CL2)      ADC.CH2     ADC2_IN5    (PC4)
// HV_AMP          ADC.CH10    ADC2_IN13   (PA5)

// ADC channel mappings (ADC3)
// TEMP.T1  -> ADC3_IN4  (PE7)
// TEMP.T2  -> ADC3_IN6  (PE8)
// TEMP.T3  -> ADC3_IN14 (PE10)
// TEMP.T4  -> ADC3_IN2  (PE9)
// TEMP.T5  -> ADC3_IN15 (PE11)
// TEMP.T7  -> ADC3_IN16 (PE12)
// TEMP.T8  -> ADC3_IN3  (PE13)

// ADC channel mappings (ADC4)
// TEMP.T6  -> ADC4_IN1  (PE14)

// ADC channel mappings (ADC5)
// PWR      -> ADC5_IN2  (PA9)

volatile uint16_t adc1_dma_buffer[ADC1_NUM_CHANNELS];

void adc_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC12EN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC345EN);
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN);


    // Initialize GPIO clocks
    gpio_init();
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();

    // Configure analog inputs
    gpio_set_analog(GPIOA, 0);    // ADC.CH1
    gpio_set_analog(GPIOC, 4);    // ADC.CH2
    gpio_set_analog(GPIOC, 2);    // ADC.CH3
    gpio_set_analog(GPIOA, 3);    // ADC.CH4
    gpio_set_analog(GPIOA, 4);    // ADC.CH5
    gpio_set_analog(GPIOA, 1);    // ADC.CH6
    gpio_set_analog(GPIOC, 1);    // ADC.CH7
    gpio_set_analog(GPIOC, 3);    // ADC.CH8
    gpio_set_analog(GPIOA, 2);    // ADC.CH9
    gpio_set_analog(GPIOA, 5);    // ADC.CH10

    gpio_set_analog(GPIOE, 7);    // TEMP.T1
    gpio_set_analog(GPIOE, 8);    // TEMP.T2
    gpio_set_analog(GPIOE, 10);   // TEMP.T3
    gpio_set_analog(GPIOE, 9);    // TEMP.T4
    gpio_set_analog(GPIOE, 11);   // TEMP.T5
    gpio_set_analog(GPIOE, 14);   // TEMP.T6
    gpio_set_analog(GPIOE, 12);   // TEMP.T7
    gpio_set_analog(GPIOE, 13);   // TEMP.T8

    gpio_set_analog(GPIOA, 9);    // PWR

    // ADC clock mode is adc_hclk
    MODIFY_REG(ADC12_COMMON->CCR, ADC_CCR_CKMODE_Msk, ADC_CCR_CKMODE_0);
    MODIFY_REG(ADC345_COMMON->CCR, ADC_CCR_CKMODE_Msk, ADC_CCR_CKMODE_0);
    
    // RM0440 p. 603 - ADC DEEPPWD and ADVREGEN
    // 1. Exit Deep-power-down mode by clearing DEEPPWD bit
    CLEAR_BIT(ADC1->CR, ADC_CR_DEEPPWD);
    CLEAR_BIT(ADC2->CR, ADC_CR_DEEPPWD);
    CLEAR_BIT(ADC3->CR, ADC_CR_DEEPPWD);
    CLEAR_BIT(ADC4->CR, ADC_CR_DEEPPWD);
    CLEAR_BIT(ADC5->CR, ADC_CR_DEEPPWD);
    // 2. Enable the ADC voltage regulator by setting ADVREGEN
    SET_BIT(ADC1->CR, ADC_CR_ADVREGEN);
    SET_BIT(ADC2->CR, ADC_CR_ADVREGEN);
    SET_BIT(ADC3->CR, ADC_CR_ADVREGEN);
    SET_BIT(ADC4->CR, ADC_CR_ADVREGEN);
    SET_BIT(ADC5->CR, ADC_CR_ADVREGEN);
    // 3. Wait for the startup time to configure the ADC
    for (volatile uint16_t i = 0; i < 1000; i++) __NOP();

    // RM0440 p. 605 - Software procedure to calibrate the ADC
    // 1. Ensure DEEPPWD = 0, ADVREGEN = 1 and that ADC voltage regulator startup time has elapsed
    // 2. Ensure that ADEN = 0
    CLEAR_BIT(ADC1->CR, ADC_CR_ADEN);
    CLEAR_BIT(ADC2->CR, ADC_CR_ADEN);
    CLEAR_BIT(ADC3->CR, ADC_CR_ADEN);
    CLEAR_BIT(ADC4->CR, ADC_CR_ADEN);
    CLEAR_BIT(ADC5->CR, ADC_CR_ADEN);
    // 3. Select the input mode for this calibration by setting ADCALDIF = 0 (single-ended input) or ADCALDIF = 1 (differential input)
    // 4. Set ADCAL
    SET_BIT(ADC1->CR, ADC_CR_ADCAL);
    SET_BIT(ADC2->CR, ADC_CR_ADCAL);
    SET_BIT(ADC3->CR, ADC_CR_ADCAL);
    SET_BIT(ADC4->CR, ADC_CR_ADCAL);
    SET_BIT(ADC5->CR, ADC_CR_ADCAL);
    // 5. Wait until ADCAL = 0
    while (READ_BIT(ADC1->CR, ADC_CR_ADCAL) || 
           READ_BIT(ADC2->CR, ADC_CR_ADCAL) || 
           READ_BIT(ADC3->CR, ADC_CR_ADCAL) ||
           READ_BIT(ADC4->CR, ADC_CR_ADCAL) ||
           READ_BIT(ADC5->CR, ADC_CR_ADCAL));
    
    // 6. The calibration factor can be read from ADC_CALFACT register
    // The calibration data is automatically stored in the ADC registers so it is not required by the user to write it back into the peripheral

    // Configure ADC
    // 12-bit resolution
    CLEAR_BIT(ADC1->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    CLEAR_BIT(ADC2->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    CLEAR_BIT(ADC3->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    CLEAR_BIT(ADC4->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    CLEAR_BIT(ADC5->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    // Continuous conversion mode
    CLEAR_BIT(ADC1->CFGR, ADC_CFGR_DISCEN);
    SET_BIT(ADC1->CFGR, ADC_CFGR_CONT);
    CLEAR_BIT(ADC2->CFGR, ADC_CFGR_DISCEN);
    SET_BIT(ADC2->CFGR, ADC_CFGR_CONT);
    CLEAR_BIT(ADC3->CFGR, ADC_CFGR_DISCEN);
    SET_BIT(ADC3->CFGR, ADC_CFGR_CONT);
    CLEAR_BIT(ADC4->CFGR, ADC_CFGR_DISCEN);
    SET_BIT(ADC4->CFGR, ADC_CFGR_CONT);
    CLEAR_BIT(ADC5->CFGR, ADC_CFGR_DISCEN);
    SET_BIT(ADC5->CFGR, ADC_CFGR_CONT);
    // Oversampling 256x
    SET_BIT(ADC1->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC1->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC1->CFGR2, ADC_CFGR2_ROVSE);
    SET_BIT(ADC2->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC2->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC2->CFGR2, ADC_CFGR2_ROVSE);
    SET_BIT(ADC3->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC3->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC3->CFGR2, ADC_CFGR2_ROVSE);
    SET_BIT(ADC4->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC4->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC4->CFGR2, ADC_CFGR2_ROVSE);
    SET_BIT(ADC5->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC5->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC5->CFGR2, ADC_CFGR2_ROVSE);
    // Software trigger
    CLEAR_BIT(ADC1->CFGR, ADC_CFGR_EXTEN);
    CLEAR_BIT(ADC2->CFGR, ADC_CFGR_EXTEN);
    CLEAR_BIT(ADC3->CFGR, ADC_CFGR_EXTEN);
    CLEAR_BIT(ADC4->CFGR, ADC_CFGR_EXTEN);
    CLEAR_BIT(ADC5->CFGR, ADC_CFGR_EXTEN);

    // Configure sampling time for channels (640.5 ADC clock cycles for all channels,
    // SMPPLUS is set to 0)
    MODIFY_REG(ADC1->SMPR1, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC1->SMPR2, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC2->SMPR1, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC2->SMPR2, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC3->SMPR1, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC3->SMPR2, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC4->SMPR1, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC4->SMPR2, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC5->SMPR1, 0xFFFFFFFF, 0x3FFFFFFF);
    MODIFY_REG(ADC5->SMPR2, 0xFFFFFFFF, 0x3FFFFFFF);


    // Regular channel sequence length = 8 for ADC1
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_L_Msk, ((ADC1_NUM_CHANNELS-1) << ADC_SQR1_L_Pos));
    // Configure channel sequence order Configure channel sequence order
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ1_Msk,  (1  << ADC_SQR1_SQ1_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ2_Msk,  (8  << ADC_SQR1_SQ2_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ3_Msk,  (4  << ADC_SQR1_SQ3_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ4_Msk,  (6  << ADC_SQR1_SQ4_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ5_Msk,  (2  << ADC_SQR2_SQ5_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ6_Msk,  (7  << ADC_SQR2_SQ6_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ7_Msk,  (9  << ADC_SQR2_SQ7_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ8_Msk,  (3  << ADC_SQR2_SQ8_Pos));

    



    // ***** Claude ******

    // --- DMA + DMAMUX Configuration for ADC1 ---

    // 1. Enable DMA1 clock (see above)

    // 2. Configure DMAMUX1 Channel 0 to route ADC1 -> DMA1 Channel 1
    //    ADC1 DMAMUX request ID = 5 on STM32G4 (RM0440 Table 91)
    MODIFY_REG(DMAMUX1_Channel0->CCR, DMAMUX_CxCR_DMAREQ_ID_Msk, (5 << DMAMUX_CxCR_DMAREQ_ID_Pos));

    // 3. Configure DMA1 Channel 1
    //    - Disable channel before configuring
    CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);

    //    - Peripheral address = ADC1 data register
    DMA1_Channel1->CPAR = (uint32_t)(&ADC1->DR);

    //    - Memory address = our result buffer
    DMA1_Channel1->CMAR = (uint32_t)(adc1_dma_buffer);

    //    - Number of data items to transfer
    DMA1_Channel1->CNDTR = ADC1_NUM_CHANNELS;

    //    - Configure the channel:
    //      * Circular mode (auto-reload CNDTR after each full sequence)
    //      * Memory increment (advance buffer pointer after each transfer)
    //      * Peripheral size = 16-bit (ADC DR is 16-bit)
    //      * Memory size = 16-bit
    //      * Direction = peripheral to memory
    //      * Priority = high
    MODIFY_REG(DMA1_Channel1->CCR, 
        DMA_CCR_CIRC  |     // Circular mode
        DMA_CCR_MINC  |     // Memory increment
        DMA_CCR_PSIZE |     // Peripheral data size (clear first)
        DMA_CCR_MSIZE |     // Memory data size (clear first)
        DMA_CCR_DIR   |     // Direction (0 = periph-to-mem)
        DMA_CCR_PL,         // Priority (clear first)
        DMA_CCR_CIRC  |
        DMA_CCR_MINC  |
        DMA_CCR_PSIZE_0 |   // 01 = 16-bit peripheral
        DMA_CCR_MSIZE_0 |   // 01 = 16-bit memory
        DMA_CCR_PL_1        // 10 = High priority
    );

    //    - Enable DMA1 Channel 1
    SET_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);

    // 4. Enable DMA mode in ADC + circular DMA (so DMA requests continue after sequence end)
    SET_BIT(ADC1->CFGR, ADC_CFGR_DMAEN);
    SET_BIT(ADC1->CFGR, ADC_CFGR_DMACFG);  // Circular DMA mode




    // ***************

    // RM0440 p. 607 - Software procedure to enable the ADC
    // 1. Clear the ADRDY bit in the ADC_ISR register by writing 1
    ADC1->ISR |= ADC_ISR_ADRDY;
    // 2. Set ADEN
    ADC1->CR |= ADC_CR_ADEN;
    // 3. Wait until ADRDY = 1
    while (!(ADC1->ISR & ADC_ISR_ADRDY));

    // Start conversions
    SET_BIT(ADC1->CR, ADC_CR_ADSTART);
}