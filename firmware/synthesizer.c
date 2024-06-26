#include "synthesizer.h"

#include <stm32f072xb.h>

// Buffer that contains the calibrated waveform
uint16_t waveform[SYNTH_WAVEFORM_N];

void synth_setup()
{
	const int step_size = 4096 / SYNTH_WAVEFORM_N;
	uint16_t val = 0;
	// Setup the waveform to the sane default staircase
	for(int i = 0; i < SYNTH_WAVEFORM_N; i++)
	{
		waveform[i] = val;
		val += step_size;
	}

	// DAC is triggered by TRGO, DMA channel 3 is triggered by DAC at the same time,
	// Setup and load the prescaler, this makes the clock tick at 24MHz
	// (Divide 24Mhz by this number)
	// Must be more than 3 because otherwise DMA cannot keep up
	TIM6->PSC = 5;
	// Make sure we auto-reload the preload
	TIM6->CR1 |= TIM_CR1_ARPE;
	// We want TRGO to be after the prescaler (ie in update)
	TIM6->CR2 |= TIM_CR2_MMS_1;
	// Finally, update will happen every time this counter reaches 0, ie,
	// this sets the frequency by dividing by this number
	// (Note that generated signal frequency is further divided by N samples)
	TIM6->ARR = 40;

	// The freq of the generated signal is thus 24Mhz / PSC / ARR / N_samples
	// (Approximately, this thing for wathever reason is rough!)
	// TODO: Check the math, and explain why it doesn't match

	// Point DMA to the DAC (We use 12 bit right aligned), lower 16 bits
	DMA1_Channel3->CPAR = (uint32_t)(&DAC->DHR12RD);
	// Point DMA to the buffer
	DMA1_Channel3->CMAR = (uint32_t)(&waveform);
	// Make it transfer the whole buffer
	DMA1_Channel3->CNDTR = SYNTH_WAVEFORM_N; // NUMBER of transfers, not bytes
	// Setup DMA to transfer a byte from waveform to the DAC on each trigger
	// 16 bit transfer to 16 bit peripheral, memory increment, circular
	DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_DIR;
	DMA1_Channel3->CCR |= DMA_CCR_TCIE;

	// Setup the DAC
	// Make it trigger the DMA and be triggered by triggers (TIM6)
	DAC1->CR |= DAC_CR_DMAUDRIE1 | DAC_CR_DMAEN1 | DAC_CR_TEN1;

	// SHDN pin of the VCO set to output
	GPIOB->MODER |= (1 << GPIO_MODER_MODER4_Pos);
	vco_set_running(false);
}

void synth_interrupt()
{
}

void vco_set_running(bool value)
{
	if(value)
		GPIOB->ODR |= GPIO_ODR_4;
	else
		GPIOB->ODR &= ~GPIO_ODR_4;
}

void synth_start()
{
	// Start the clock
	TIM6->EGR |= TIM_EGR_UG;
	TIM6->CR1 |= TIM_CR1_CEN;
	// Start the DMA
	DMA1_Channel3->CCR |= DMA_CCR_EN;
	// Start the DAC
	DAC1->CR |= DAC_CR_EN1;

}
