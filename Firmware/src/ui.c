#include "ui.h"

volatile const uint8_t  BTN_HOLD_THRESH = 15;
volatile const uint32_t BTN_DB_TIME_MS  = 50;

/*
Main screens
*******************************************************
*   V: XXX.XX V | XXX.XX   *   V: XXX.XX V |  X.XXX   *
*   I:  X.XXX A |      W   *   I:  X.XXX A |      A   *
*   P: XXX.XX W | B 100%   *   P: XXX.XX W | B 100%   *
*   M: CP    0-500V,0-1A   *   M: CC    0-500V,0-1A   *
*******************************************************
*   V: XXX.XX V | XXX.XX   *   V: XXX.XX V |  XXXXX   *
*   I:  X.XXX A |      V   *   I:  X.XXX A |      Ω   *
*   P: XXX.XX W | B 100%   *   P: XXX.XX Ω | B 100%   *
*   M: CV    0-500V,0-1A   *   M: CR    0-500V,0-1A   *
*******************************************************
*/

volatile const float CP_CURSOR_LIM_UPR = 100.0;     // XXX.XX  W
volatile const float CC_CURSOR_LIM_UPR = 1.0;       // X.XXX   A
volatile const float CV_CURSOR_LIM_UPR = 100.0;     // XXX.XX  V
volatile const float CR_CURSOR_LIM_UPR = 10000.0;   // XXXXX   Ω

volatile const float CP_CURSOR_LIM_LWR = 0.01;      // XXX.XX  W
volatile const float CC_CURSOR_LIM_LWR = 0.001;     // X.XXX   A
volatile const float CV_CURSOR_LIM_LWR = 0.01;      // XXX.XX  V
volatile const float CR_CURSOR_LIM_LWR = 1.0;       // XXXXX   Ω

GPIO_TypeDef * BTN_PORTS[8] = { GPIOC, GPIOC, GPIOC, GPIOC, GPIOD, GPIOC, GPIOC, GPIOC };
volatile const uint8_t BTN_PINS[8]  = { 8, 7, 6, 9, 0, 12, 11, 10 };

volatile uint8_t ui_tmr;
volatile uint8_t ui_btn[8] = { 0 };

volatile uint8_t btn_state[8] = { 0 };
volatile uint8_t btn_state_old[8] = { 0 };
volatile uint8_t btn_hold_cnt[8] = { 0 };
volatile uint8_t last_btn_irq_tick[8] = { 0 };

// Store the current state of the UI
volatile float cursorPosition = 0.1;
volatile float voltageSet = 0.0;
volatile float currentSet = 0.0;
volatile float resistanceSet = 0.0;
volatile float powerSet = 0.0;
volatile uint8_t cursorOn = 0;
volatile uint8_t loadOn = 0;
Modes currentMode = CONSTANT_CURRENT;

void ui_init(void)
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

    // Only start lptim (which keeps track of buttons holds), after the buttons' interrupts have been set up
    lptim_init();
}

void ui_poll(void)
{
    if (ui_tmr > 0)
    {
        // Time has elapsed since last ui_poll()
        ui_tmr--;

        for (uint8_t b = 0; b < 8; b++)
        {
            // Button currently pressed and was pressed last time
            if (btn_state[b] == 1 && btn_state_old[b] == 1)
            {
                if (btn_hold_cnt[b] < BTN_HOLD_THRESH)
                {
                    btn_hold_cnt[b]++;
                }
            }

            // Button is not pressed
            if (btn_state[b] == 0)
            {
                btn_hold_cnt[b] = 0;
                gpio_reset(GPIOE, 1);
            }

            //  Button has been held down for enough time
            if (btn_hold_cnt[b] == BTN_HOLD_THRESH)
            {
                // Perform button hold action
                ui_event(b);
            }

            btn_state_old[b] = btn_state[b];
        }
    }

    for (uint8_t b = 0; b < 8; b++)
    {
        if (ui_btn[b] > 0)
        {
            // Button was pressed at least once since last ui_poll()
            ui_btn[b]--;

            // Perform button pressed action
            ui_event(b);
        }
    }
}

void ui_event(uint8_t b)
{
    // Will be called for each button press, if it is being held down this event fires repeatedly
    // Button layout:
    // 0                       4
    // 1                       5
    // 2                       6
    // 3                       7

    // Modify variables as appropriate
    switch (b)
    {
        case 0:
            // Move cursor left one position
            if (cursorPosition < CP_CURSOR_LIM_UPR && currentMode == CONSTANT_POWER)
                cursorPosition *= 10;
            if (cursorPosition < CC_CURSOR_LIM_UPR && currentMode == CONSTANT_CURRENT)
                cursorPosition *= 10;
            if (cursorPosition < CV_CURSOR_LIM_UPR && currentMode == CONSTANT_VOLTAGE)
                cursorPosition *= 10;
            if (cursorPosition < CR_CURSOR_LIM_UPR && currentMode == CONSTANT_RESISTANCE)
                cursorPosition *= 10;
            break;
        case 1:
            // Move cursor right one position
            if (cursorPosition > CP_CURSOR_LIM_LWR && currentMode == CONSTANT_POWER)
                cursorPosition /= 10;
            if (cursorPosition > CC_CURSOR_LIM_LWR && currentMode == CONSTANT_CURRENT)
                cursorPosition /= 10;
            if (cursorPosition > CV_CURSOR_LIM_LWR && currentMode == CONSTANT_VOLTAGE)
                cursorPosition /= 10;
            if (cursorPosition > CR_CURSOR_LIM_LWR && currentMode == CONSTANT_RESISTANCE)
                cursorPosition /= 10;
            break;
        case 2:
            // Increase digit
            if (cursorOn && currentMode == CONSTANT_POWER)
                powerSet += cursorPosition;
            if (cursorOn && currentMode == CONSTANT_CURRENT)
                currentSet += cursorPosition;
            if (cursorOn && currentMode == CONSTANT_VOLTAGE)
                voltageSet += cursorPosition;
            if (cursorOn && currentMode == CONSTANT_RESISTANCE)
                resistanceSet += cursorPosition;
            break;
        case 3:
            // Decrease digit
            if (cursorOn && currentMode == CONSTANT_POWER)
                powerSet -= cursorPosition;
            if (cursorOn && currentMode == CONSTANT_CURRENT)
                currentSet -= cursorPosition;
            if (cursorOn && currentMode == CONSTANT_VOLTAGE)
                voltageSet -= cursorPosition;
            if (cursorOn && currentMode == CONSTANT_RESISTANCE)
                resistanceSet -= cursorPosition;
            break;
        case 4:
            // Toggle mode (CP/CC/CV/CR)
            switch (currentMode)
            {
                case CONSTANT_POWER:
                    currentMode = CONSTANT_CURRENT;
                    break;
                case CONSTANT_CURRENT:
                    currentMode = CONSTANT_VOLTAGE;
                    break;
                case CONSTANT_VOLTAGE:
                    currentMode = CONSTANT_RESISTANCE;
                    break;
                case CONSTANT_RESISTANCE:
                    currentMode = CONSTANT_POWER;
                    break;
                default:
                    break;
            }
            break;
        case 5:
            // Reserved for future use
            break;
        case 6:
            // Cursor on/off
            if (cursorOn == 0)
                cursorOn = 1;
            else
                cursorOn = 0;
            break;
        case 7:
            // Load on/off
            if (loadOn == 0)
                loadOn = 1;
            if (loadOn == 1)
                loadOn = 0;
            break;
        default:
            break;
    }
    
    // Update the display
    switch (currentMode)
    {
        case CONSTANT_POWER:
            sprintf((char *)lcd_l1, "V: %6.2f V | %6.2f", voltageMeas, powerSet);
            sprintf((char *)lcd_l2, "I: %5.3f A |      W", currentMeas);
            sprintf((char *)lcd_l3, "%6.2f W | B %3d%%", (voltageMeas * currentMeas), batteryPercentage);
            sprintf((char *)lcd_l4, "CP    0-500V,0-1A");
            break;
        case CONSTANT_CURRENT:
            sprintf((char *)lcd_l1, "V: %6.2f V | %5.3f", voltageMeas, currentSet);
            sprintf((char *)lcd_l2, "I: %5.3f A |      W", currentMeas);
            sprintf((char *)lcd_l3, "%6.2f W | B %3d%%", (voltageMeas * currentMeas), batteryPercentage);
            sprintf((char *)lcd_l4, "CC    0-500V,0-1A");
            break;
        case CONSTANT_VOLTAGE:
            sprintf((char *)lcd_l1, "V: %6.2f V | %6.2f", voltageMeas, voltageSet);
            sprintf((char *)lcd_l2, "I: %5.3f A |      W", currentMeas);
            sprintf((char *)lcd_l3, "%6.2f W | B %3d%%", (voltageMeas * currentMeas), batteryPercentage);
            sprintf((char *)lcd_l4, "CV    0-500V,0-1A");
            break;
        case CONSTANT_RESISTANCE:
            sprintf((char *)lcd_l1, "V: %6.2f V | %5d", voltageMeas, resistanceSet);
            sprintf((char *)lcd_l2, "I: %5.3f A |      W", currentMeas);
            sprintf((char *)lcd_l3, "%6.2f W | B %3d%%", (voltageMeas * currentMeas), batteryPercentage);
            sprintf((char *)lcd_l4, "CR    0-500V,0-1A");
            break;
        default:
            break;
    }
}

__attribute__((always_inline)) static inline void btn_handler(uint8_t b)
{
    if ((SysTickCounter - last_btn_irq_tick[b]) > BTN_DB_TIME_MS)
    {
        last_btn_irq_tick[b] = SysTickCounter;
        uint8_t pressed = 1 - gpio_read(BTN_PORTS[b], BTN_PINS[b]);
        if (pressed && btn_state[b] == 0)
        {
            ui_btn[b]++;
            gpio_set(GPIOE, 1);
        }
        btn_state[b] = pressed;
    }
}

void EXTI0_IRQHandler(void)
{
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF0))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF0);
        btn_handler(4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF6))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF6);
        btn_handler(2);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF7))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF7);
        btn_handler(1);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF8))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF8);
        btn_handler(0);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF9))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF9);
        btn_handler(3);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF10))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF10);
        btn_handler(7);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF11))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF11);
        btn_handler(6);
    }
    if (READ_BIT(EXTI->PR1, EXTI_PR1_PIF12))
    {
        SET_BIT(EXTI->PR1, EXTI_PR1_PIF12);
        btn_handler(5);
    }
}