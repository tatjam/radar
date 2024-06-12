#include "led_control.h"
#include "stm32f0xx.h"
#include <tusb.h>

static int led_timer = 0;

void led_systick()
{
	led_timer--;
	if(led_timer < 0)
	{
		// Toggle LED
		GPIOB->ODR ^= GPIO_ODR_2;
		if(tud_connected())
		{
			// Flash every 3 seconds once USB is connected
			led_timer = 3000;
		}
		else
		{
			// Flash quickly while not USB connected
			led_timer = 500;
		}
	}



}
