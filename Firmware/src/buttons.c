#include "buttons.h"

void buttons_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();

    gpio_init();

    gpio_set_input(GPIOC, 8);       // BTN1
    gpio_set_input(GPIOC, 7);       // BTN2
    gpio_set_input(GPIOC, 6);       // BTN3
    gpio_set_input(GPIOC, 9);       // BTN4
    gpio_set_input(GPIOD, 0);       // BTN5
    gpio_set_input(GPIOC, 12);      // BTN6
    gpio_set_input(GPIOC, 11);      // BTN7
    gpio_set_input(GPIOC, 10);      // BTN8

    // RM0440 p. 443 - Hardware interrupt selection
    // 1. Configure the corresponding mask bit in the EXTI_IMR register
    SET_BIT(EXTI->IMR1, EXTI_IMR1_IM0 |
                        EXTI_IMR1_IM6 |
                        EXTI_IMR1_IM7 |
                        EXTI_IMR1_IM8 |
                        EXTI_IMR1_IM9 |
                        EXTI_IMR1_IM10 |
                        EXTI_IMR1_IM11 |
                        EXTI_IMR1_IM12);
    // 2. Configure the Trigger Selection bits of the Interrupt line (EXTI_RTSR and EXTI_FTSR)
    SET_BIT(EXTI->RTSR1, EXTI_RTSR1_RT0 |
                         EXTI_RTSR1_RT6 |
                         EXTI_RTSR1_RT7 |
                         EXTI_RTSR1_RT8 |
                         EXTI_RTSR1_RT9 |
                         EXTI_RTSR1_RT10 |
                         EXTI_RTSR1_RT11 |
                         EXTI_RTSR1_RT12);
    SET_BIT(EXTI->FTSR1, EXTI_FTSR1_FT0 |
                         EXTI_FTSR1_FT6 |
                         EXTI_FTSR1_FT7 |
                         EXTI_FTSR1_FT8 |
                         EXTI_FTSR1_FT9 |
                         EXTI_FTSR1_FT10 |
                         EXTI_FTSR1_FT11 |
                         EXTI_FTSR1_FT12);
    // 3. Configure the enable and mask bits that control the NVIC IRQ channel mapped to the EXTI
    //    so that an interrupt coming from one of the EXTI lines can be correctly acknowledged
    MODIFY_REG(SYSCFG->EXTICR[0], SYSCFG_EXTICR1_EXTI0,  SYSCFG_EXTICR1_EXTI0_PD);
    MODIFY_REG(SYSCFG->EXTICR[1], SYSCFG_EXTICR2_EXTI6,  SYSCFG_EXTICR2_EXTI6_PC);
    MODIFY_REG(SYSCFG->EXTICR[1], SYSCFG_EXTICR2_EXTI7,  SYSCFG_EXTICR2_EXTI7_PC);
    MODIFY_REG(SYSCFG->EXTICR[2], SYSCFG_EXTICR3_EXTI8,  SYSCFG_EXTICR3_EXTI8_PC);
    MODIFY_REG(SYSCFG->EXTICR[2], SYSCFG_EXTICR3_EXTI9,  SYSCFG_EXTICR3_EXTI9_PC);
    MODIFY_REG(SYSCFG->EXTICR[2], SYSCFG_EXTICR3_EXTI10, SYSCFG_EXTICR3_EXTI10_PC);
    MODIFY_REG(SYSCFG->EXTICR[2], SYSCFG_EXTICR3_EXTI11, SYSCFG_EXTICR3_EXTI11_PC);
    MODIFY_REG(SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI12, SYSCFG_EXTICR4_EXTI12_PC);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void buttons_poll(void)
{
    if (buttons_trg_tmr > 0)
    {
        buttons_trg_tmr--;
    }

    if (buttons_trg_btn[0] > 0) {
        buttons_trg_btn[1]--;
    }

    if (buttons_trg_btn[2] > 0) {
        buttons_trg_btn[2]--;
    }

    if (buttons_trg_btn[3] > 0) {
        buttons_trg_btn[3]--;
    }

    if (buttons_trg_btn[4] > 0) {
        buttons_trg_btn[4]--;
    }

    if (buttons_trg_btn[5] > 0) {
        buttons_trg_btn[5]--;
    }

    if (buttons_trg_btn[6] > 0) {
        buttons_trg_btn[6]--;
    }

    if (buttons_trg_btn[7] > 0) {
        buttons_trg_btn[7]--;
    }

    if (buttons_trg_btn[8] > 0) {
        buttons_trg_btn[8]--;
    }
}

void EXTI0_IRQHandler(void)
{
    // TODO
}

void EXTI9_5_IRQHandler(void)
{
    // TODO
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF6)) {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF6);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF7)) {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF7);
    }
}

void EXTI15_10_IRQHandler(void)
{
    // TODO
}