#include <stdint.h>
#include "stm32f072xb.h"
#include <stdbool.h>
#include "usb.h"

void enable_debug_mco()
{
    // PA8 is the MCO on this micro, enable it
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Dummy reads, as explained in silicon errata
    volatile uint32_t dummy;
    dummy = RCC->AHBENR;
    dummy = RCC->AHBENR;

    // (First of all clear bits, just in case!)
    GPIOA->MODER &= ~GPIO_MODER_MODER8;
    // Set PA8 to be alternate function
    // By default alternate function is MCO so this is enough
    GPIOA->MODER |= GPIO_MODER_MODER8_1;

    // Set MCO to output the PLL clock (so we can see it on the oscilloscope)
    // It's divided by 32 (16 * 2 default PLL post-scale) so we should see a 1.5MHz signal
    RCC->CFGR |= RCC_CFGR_MCOPRE_DIV16;
    RCC->CFGR |= RCC_CFGR_MCO_PLL;

}

void enable_external_clock()
{
    // Enable and set priority of RCC_CRS interrupt
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    // Enable clock security (NMI on clock failure)
    // Enable HSE clock
    RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEON;
    // Wait for HSE ready
    while(!(RCC->CR & RCC_CR_HSERDY)) {}

    // Configure PLL
    // Use HSE (8Mhz) divided by PREDIV (no division by default)
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
    // Multiplied by 6 it's 48MHz (USB)
    RCC->CFGR |= RCC_CFGR_PLLMUL6;
    // Enable the PLL
    RCC->CR |= RCC_CR_PLLON;

    // Wait for PLL ready
    while(!(RCC->CR & RCC_CR_PLLRDY)) {}

    // Set AHB prescaler to divide by 2, so AHB runs at 24MHz
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV2;

    // Set system clock to use PLL (as fast as it can go infact)
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Set USB to use the PLL
    RCC->CFGR3 |= RCC_CFGR3_USBSW_PLLCLK;

    enable_debug_mco();
}


void set_led(bool on)
{
    if(on)
        GPIOB->ODR |= GPIO_ODR_2;
    else
        GPIOB->ODR &= ~GPIO_ODR_2;
}


int main()
{
    enable_external_clock(); 
    enable_usb();

    // Enable GPIOC peripheral
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN_Msk;

    // Dummy reads, as explained in silicon errata
    volatile uint32_t dummy;
    dummy = RCC->AHBENR;
    dummy = RCC->AHBENR;

    // Enable PIN 13 for output
    GPIOB->MODER |= (1 << GPIO_MODER_MODER2_Pos);
    GPIOB->ODR |= GPIO_ODR_2;

    while(true)
    {
        set_led(true);
        // Busy loop
        for(uint32_t i = 0; i < 1000000; i++)
        {
            
        }
        set_led(false);
        for(uint32_t i = 0; i < 1000000; i++)
        {
            
        } 

    }

}
