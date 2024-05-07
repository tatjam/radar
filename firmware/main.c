#include <stdint.h>
#include "stm32f072xb.h"
#include <stdbool.h>

volatile bool external_clock_ready;

void rcc_crs_handler()
{
    // Check that the interrupt was called because HSE is ready
    if((RCC->CIR & RCC_CIR_HSERDYF) != 0)
    {
        // Clear interrupt flag, as we have handled it
        RCC->CIR |= RCC_CIR_HSERDYC;
        // Set to 01 which means use HSE
        RCC->CFGR |= RCC_CFGR_SW_0;
        // Report to main so it exits its busy loop
        external_clock_ready = true;
    }
}

void enable_external_clock()
{
    // Enable and set priority of RCC_CRS interrupt
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn, 0);

    // We want to be interrupted if HSE (high speed external clock) is ready
    RCC->CIR |= RCC_CIR_HSERDYIE;
    // Enable clock security (NMI on clock failure)
    // Enable bypass HSE oscillator with external clock
    // Enable HSE clock
    RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEON;
    // Once HSE is ready, the interrupt will be dispatched
    
}


int main()
{
    external_clock_ready = false;
    enable_external_clock(); 
    //__enable_irq();
    // Wait for clock to come up...
    while(!external_clock_ready) {}

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
        GPIOB->ODR ^= GPIO_ODR_2;
        // Busy loop
        for(uint32_t i = 0; i < 100000; i++)
        {
            
        }

    }

}
