#include "gpio.h"

void gpio_init(void)
{
    // RM0440 p. 285 - When the peripheral clock is not active, the peripheral registers read or write access is not supported
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN);
    SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOGEN);
    // RM0440 p. 285 - Just after enabling the clock for a peripheral, software must wait for a delay before accessing the peripheral registers
    __DSB();
}

void gpio_set_output(GPIO_TypeDef * port, uint16_t pin)
{
    MODIFY_REG(port->MODER, (0b11 << (pin * 2)), (0b01 << (pin * 2)));
}

void gpio_set_input(GPIO_TypeDef * port, uint16_t pin)
{
    MODIFY_REG(port->MODER, (0b11 << (pin * 2)), (0b00 << (pin * 2)));
}

void gpio_set_alt(GPIO_TypeDef * port, uint16_t pin, uint8_t alt_mode)
{
    // Catch for invalid pin number
    if (pin > 15) return;

    // Set MODER to alternate function
    uint32_t shift = pin * 2;
    uint32_t mask = 0x3 << shift;
    uint32_t set = 0b10 << shift;
    MODIFY_REG(port->MODER, mask, set);

    // Set the alternate function using either AFRL or AFRH registers
    if (pin > 7 && pin < 16)
    {
        shift = ((pin - 8) * 4);
        mask = 0xF << shift;
        set = alt_mode << shift;
        MODIFY_REG(port->AFR[1], mask, set);
    }
    else
    if (pin < 8)
    {
        shift = (pin * 4);
        mask = 0xF << shift;
        set = alt_mode << shift;
        MODIFY_REG(port->AFR[0], mask, set);
    }
    else
    {
        // Invalid pin number
    }
}

void gpio_set(GPIO_TypeDef * port, uint16_t pin)
{
    SET_BIT(port->ODR, (1 << pin));
}

void gpio_reset(GPIO_TypeDef * port, uint16_t pin)
{
    CLEAR_BIT(port->ODR, (1 << pin));
}

void gpio_write(GPIO_TypeDef * port, uint16_t pin, uint8_t state)
{
    if (state) gpio_set(port, pin);
    else       gpio_reset(port, pin);
}

uint8_t gpio_read(GPIO_TypeDef * port, uint16_t pin)
{
    if (port->IDR & (1 << pin)) return 1;
    return 0;
}