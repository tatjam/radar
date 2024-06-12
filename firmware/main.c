#include <stdint.h>
#include "class/cdc/cdc_device.h"
#include "stm32f0xx.h"
#include <stdbool.h>
#include <tusb.h>

int64_t abs_time;

static void enable_debug_mco()
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

static void enable_external_clock()
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

static void enable_usb()
{
	// Enable USB peripheral
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;
	// And its GPIOA
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Enable USB interrupt
	NVIC_SetPriority(USB_IRQn, 8);
	NVIC_EnableIRQ(USB_IRQn);

	// Enable USB macrocell (exit power down mode)
	USB->CNTR &= ~USB_CNTR_PDWN;

	// Wait a few microseconds for clock stability
	for(uint32_t i = 0; i < 10000; i++)
	{
		
	}

	// Enable reset and correct transfer interrupts
	USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM;
	// Enable pull up on D+ to signal that we are there
	USB->BCDR |= USB_BCDR_DPPU;

	// Stop resetting the peripheral so USB operation may begin
	// (this flag is set by default)
	USB->CNTR &= ~USB_CNTR_FRES;
}

static void enable_adc()
{

}


void systick_handler()
{
	static bool last_state = false;
	abs_time++;
}

int main()
{
	enable_external_clock();
	enable_usb();
	enable_adc();

	// Enable GPIOC peripheral
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN_Msk;

	// Dummy reads, as explained in silicon errata
	volatile uint32_t dummy;
	dummy = RCC->AHBENR;
	dummy = RCC->AHBENR;

	// Enable PIN 13 for output
	GPIOB->MODER |= (1 << GPIO_MODER_MODER2_Pos);
	GPIOB->ODR |= GPIO_ODR_2;

	abs_time = 0;
	// TODO:
	// This should be set to 24000 by the manual (AHB prescaler is 1/2), but
	// for wathever reason we need 12000. Maybe clocks are wrong?
	SysTick->LOAD = 12000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	tusb_init();

	while(true)
	{
		tud_task();

		if(tud_cdc_n_connected(0))
		{
			uint32_t cnt = 0;
			if(tud_cdc_available())
			{
				uint8_t buf[64];
				cnt = tud_cdc_read(buf, sizeof(buf));
				tud_cdc_write(buf, 1);
				tud_cdc_write_flush();
			}
		}
	}

}
