#include "stm32f072xb.h"
#include <stdbool.h>


int main()
{
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
