#include "led_control.h"

#include <opamp_control.h>

#include "stm32f0xx.h"
#include <tusb.h>
#include <utils.h>

static int led_timer = 0;
static uint32_t last_abstime = 0;

void led_task()
{
	const int dt = abs_time - last_abstime;
	last_abstime = abs_time;
	led_timer -= dt;

	if(led_timer < 0)
	{
		// Toggle LED
		GPIOB->ODR ^= GPIO_ODR_2;
		if(tud_mounted())
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
