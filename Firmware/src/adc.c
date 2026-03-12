#include "adc.h"

// ADC channel mappings

// I_OUT(CL1)      ADC.CH1     ADC1_IN1    (PA0)
// I_OUT(CL2)      ADC.CH2     ADC1_IN5    (PC4)
// I_OUT(CL3)      ADC.CH3     ADC1_IN8    (PC2)
// I_OUT(CL4)      ADC.CH4     ADC1_IN4    (PA3)
// I_OUT(CL5)      ADC.CH5     ADC1_IN6    (PA4)
// I_OUT(CL6)      ADC.CH6     ADC1_IN2    (PA1)
// I_OUT(CL7)      ADC.CH7     ADC1_IN7    (PC1)
// I_OUT(CL8)      ADC.CH8     ADC1_IN9    (PC3)
// LV_AMP          ADC.CH9     ADC1_IN3    (PA2)
// HV_AMP          ADC.CH10    ADC1_IN13   (PA5)

// TEMP.T1  -> ADC3_IN4  (PE7)
// TEMP.T2  -> ADC3_IN6  (PE8)
// TEMP.T3  -> ADC3_IN14 (PE13)
// TEMP.T4  -> ADC3_IN2  (PE9)
// TEMP.T5  -> ADC3_IN15 (PE11)
// TEMP.T6  -> ADC3_IN1  (PE14)
// TEMP.T7  -> ADC3_IN16 (PE12)
// TEMP.T8  -> ADC3_IN3  (PE13)

// PWR      -> ADC5_IN2  (PA9)

volatile uint16_t adc1_dma_buffer[ADC1_NUM_CHANNELS];

void adc_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC12EN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC345EN);
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
    // Initialize GPIO clocks
    gpio_init();
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();

    // Configure analog inputs
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE0_1 | GPIO_MODER_MODE0_0);    // ADC.CH1
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0);    // ADC.CH2
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODE2_1 | GPIO_MODER_MODE2_0);    // ADC.CH3
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE3_1 | GPIO_MODER_MODE3_0);    // ADC.CH4
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0);    // ADC.CH5
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE1_1 | GPIO_MODER_MODE1_0);    // ADC.CH6
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODE1_1 | GPIO_MODER_MODE1_0);    // ADC.CH7
    SET_BIT(GPIOC->MODER, GPIO_MODER_MODE3_1 | GPIO_MODER_MODE3_0);    // ADC.CH8
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE2_1 | GPIO_MODER_MODE2_0);    // ADC.CH9
    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE5_1 | GPIO_MODER_MODE5_0);    // ADC.CH10

    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE7_1 | GPIO_MODER_MODE7_0);    // TEMP.T1
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE8_1 | GPIO_MODER_MODE8_0);    // TEMP.T2
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE10_1 | GPIO_MODER_MODE10_0);  // TEMP.T3
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE9_1 | GPIO_MODER_MODE9_0);    // TEMP.T4
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE11_1 | GPIO_MODER_MODE11_0);  // TEMP.T5
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE14_1 | GPIO_MODER_MODE14_0);  // TEMP.T6
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE12_1 | GPIO_MODER_MODE12_0);  // TEMP.T7
    SET_BIT(GPIOE->MODER, GPIO_MODER_MODE13_1 | GPIO_MODER_MODE13_0);  // TEMP.T8

    SET_BIT(GPIOA->MODER, GPIO_MODER_MODE9_1 | GPIO_MODER_MODE9_0);    // PWR

    // ADC clock mode is adc_hclk
    MODIFY_REG(ADC12_COMMON->CCR, ADC_CCR_CKMODE_Msk, ADC_CCR_CKMODE_0);

    // RM0440 p. 603 - ADC DEEPPWD and ADVREGEN
    // 1. Exit Deep-power-down mode by clearing DEEPPWD bit
    CLEAR_BIT(ADC1->CR,ADC_CR_DEEPPWD);
    // 2. Enable the ADC voltage regulator by setting ADVREGEN
    SET_BIT(ADC1->CR, ADC_CR_ADVREGEN);
    // 3. Wait for the startup time to configure the ADC
    for (volatile uint16_t i = 0; i < 1000; i++) __NOP();

    // RM0440 p. 605 - Software procedure to calibrate the ADC
    // 1. Ensure DEEPPWD = 0, ADVREGEN = 1 and that ADC voltage regulator startup time has elapsed
    // 2. Ensure that ADEN = 0
    CLEAR_BIT(ADC1->CR, ADC_CR_ADEN);
    // 3. Select the input mode for this calibration by setting ADCALDIF = 0 (single-ended input) or ADCALDIF = 1 (differential input)
    // 4. Set ADCAL
    SET_BIT(ADC1->CR, ADC_CR_ADCAL);
    // 5. Wait until ADCAL = 0
    while (READ_BIT(ADC1->CR, ADC_CR_ADCAL));
    
    // 6. The calibration factor can be read from ADC_CALFACT register
    // The calibration data is automatically stored in the ADC registers so it is not required by the user to write it back into the peripheral

    // Configure ADC
    // 12-bit resolution
    CLEAR_BIT(ADC1->CFGR, ADC_CFGR_RES_1 | ADC_CFGR_RES_0);
    // Continuous conversion mode
    SET_BIT(ADC1->CFGR, ADC_CFGR_CONT);
    // Oversampling 256x
    SET_BIT(ADC1->CFGR2, ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);
    MODIFY_REG(ADC1->CFGR2, ADC_CFGR2_OVSS_Msk, (8 << ADC_CFGR2_OVSS_Pos));
    SET_BIT(ADC1->CFGR2, ADC_CFGR2_ROVSE);
    // Software trigger
    CLEAR_BIT(ADC1->CFGR, ADC_CFGR_EXTEN);

    // Configure sampling time for channels (640.5 ADC clock cycles)
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP0_Msk,  (7 << ADC_SMPR1_SMP0_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP1_Msk,  (7 << ADC_SMPR1_SMP1_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP2_Msk,  (7 << ADC_SMPR1_SMP2_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP3_Msk,  (7 << ADC_SMPR1_SMP3_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP4_Msk,  (7 << ADC_SMPR1_SMP4_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP5_Msk,  (7 << ADC_SMPR1_SMP5_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP6_Msk,  (7 << ADC_SMPR1_SMP6_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP7_Msk,  (7 << ADC_SMPR1_SMP7_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP8_Msk,  (7 << ADC_SMPR1_SMP8_Pos));
    MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP9_Msk,  (7 << ADC_SMPR1_SMP9_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP10_Msk, (7 << ADC_SMPR2_SMP10_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP11_Msk, (7 << ADC_SMPR2_SMP11_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP12_Msk, (7 << ADC_SMPR2_SMP12_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP13_Msk, (7 << ADC_SMPR2_SMP13_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP14_Msk, (7 << ADC_SMPR2_SMP14_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP15_Msk, (7 << ADC_SMPR2_SMP15_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP16_Msk, (7 << ADC_SMPR2_SMP16_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP17_Msk, (7 << ADC_SMPR2_SMP17_Pos));
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP18_Msk, (7 << ADC_SMPR2_SMP18_Pos));

    // Regular channel sequence length = 10 for ADC1
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_L_Msk, (9 << ADC_SQR1_L_Pos));
    // Configure channel sequence order
    // Order read out of ADC buffers will be ADC.CH1 -> ADC.CH10
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ1_Msk,  (1  << ADC_SQR1_SQ1_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ2_Msk,  (5  << ADC_SQR1_SQ2_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ3_Msk,  (8  << ADC_SQR1_SQ3_Pos));
    MODIFY_REG(ADC1->SQR1, ADC_SQR1_SQ4_Msk,  (4  << ADC_SQR1_SQ4_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ5_Msk,  (6  << ADC_SQR2_SQ5_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ6_Msk,  (2  << ADC_SQR2_SQ6_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ7_Msk,  (7  << ADC_SQR2_SQ7_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ8_Msk,  (9  << ADC_SQR2_SQ8_Pos));
    MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ9_Msk,  (3  << ADC_SQR2_SQ9_Pos));
    MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ10_Msk, (13 << ADC_SQR3_SQ10_Pos));





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