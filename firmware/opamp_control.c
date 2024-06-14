#include "opamp_control.h"
#include <stm32f072xb.h>
#include <utils.h>

// NOTE: The processor is slow enough that over 500ns go between instructions,
// so no explicit delays needed
void opamp_control(int steps, int choice)
{
	if(steps > 0)
	{
		// Bring U/D to high state, and then bring down CS
		GPIOA->ODR |= GPIO_ODR_9; // U/D high
	}
	else if(steps < 0)
	{
		// Bring U/D to low state, and then bring down CS
		// Any further rising edge of U/D will cause a decrease
		GPIOA->ODR &= ~GPIO_ODR_9; // U/D high
	}

	// Bring down CS
	if(choice == 1 || choice == 2)
	{
		GPIOA->ODR &= ~GPIO_ODR_7; // AMP low
	}

	if(choice == 0 || choice == 2)
	{
		GPIOA->ODR &= ~GPIO_ODR_6; // PREAMP low
	}

	// Afterwards, we bring U/D back down (or keep it there).
	// Any further rising edge of U/D will cause an increase / decrease
	GPIOA->ODR &= ~GPIO_ODR_9;

	// Now toggling U/D will cause increases / decreases
	for(int i = 0; i < ABS(steps); i++)
	{
		// Bring U/D up, which triggers a change
		GPIOA->ODR |= GPIO_ODR_9;
		// Bring U/D down so it's on a good state
		GPIOA->ODR &= ~GPIO_ODR_9;
	}


	// Bring CS back up, to prevent futher motions
	GPIOA->ODR |= GPIO_ODR_7 | GPIO_ODR_6;
}

void opamp_setup()
{
	// Just turn on both CS lines (pins A6 and A7 = 16 and 17)
	GPIOA->ODR |= GPIO_ODR_6 | GPIO_ODR_7;
}

