#include "synthesizer.h"

#include <stm32f072xb.h>

// Buffer that contains the calibrated waveform
uint16_t waveform[SYNTH_WAVEFORM_N];

void synth_setup()
{
	const int step_size = 4095 / SYNTH_WAVEFORM_N;
	uint16_t val = 0;
	int cnt = 0;
	// Setup the waveform to the sane default staircase
	for(int i = 0; i < SYNTH_WAVEFORM_N; i++)
	{
		waveform[i] = val;
		val += step_size;
	}

	// DAC is triggered by TRGO, DMA channel 3 is triggered by DAC at the same time,
	// Setup and load the prescaler, this makes the clock tick at 24MHz
	// (Divide 24Mhz by this number)
	TIM6->PSC = 0;
	// Make sure we auto-reload the preload
	TIM6->CR1 |= TIM_CR1_ARPE;
	// We want TRGO to be after the prescaler (ie in update)
	TIM6->CR2 |= TIM_CR2_MMS_1;
	// Finally, update will happen every time this counter reaches 0, ie,
	// this sets the frequency to 24MHz / (this number)
	// (Note that generated signal frequency is further divided by N samples)
	// Thus we have a DAC trigger freq from 24Mhz to 366Hz
	// and assuming 128 samples, output freq from 187.5kHz to near 0Hz
	// Default makes it be a 5.2kHz signal
	TIM6->ARR = 36;
	// TIM6->ARR = 1000;

	// Point DMA to the DAC (We use 12 bit right aligned), lower 16 bits
	DMA1_Channel3->CPAR = (uint32_t)(&DAC->DHR12RD);
	// Point DMA to the buffer
	DMA1_Channel3->CMAR = (uint32_t)(&waveform);
	// Make it transfer the whole buffer
	DMA1_Channel3->CNDTR = SYNTH_WAVEFORM_N;
	// Setup DMA to transfer a byte from waveform to the DAC on each trigger, looping around
	// 16 bit transfer to 16 bit peripheral, memory increment, circular
	DMA1_Channel3->CCR |= DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1 | DMA_CCR_MINC | DMA_CCR_CIRC;
	// We want to get notified after every sweep is complete, for system timing
	DMA1_Channel3->CCR |= DMA_CCR_TCIE;

	// Setup the DAC
	// Make it trigger the DMA and be triggered by triggers (TIM6)
	DAC1->CR |= DAC_CR_DMAUDRIE1 | DAC_CR_DMAEN1 | DAC_CR_TEN1;


}

void synth_interrupt()
{

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

	DAC1->DHR12R1 = 1000;

}
