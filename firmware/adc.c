#include "adc.h"

#include <stm32f072xb.h>

uint8_t adc_buffer[ADC_DATABUFFER];
int buffer_status;

void adc_setup()
{
	adc_calibrate();

	// Setup the ADC to use APB clock (24Mhz) divided by 2, which gives 12MHz
	// slightly below the maximum speed, but we don't really care
	//ADC1->CFGR2 |= ADC_CFGR2_CKMODE_0;
	// Setup the ADC sample-rate
	// To do so, we note that the formula to obtain the sample rate
	// is CLK / (12.5 + SMPv), where values for SMP can be found on the manual
	// Of interest are	SMPR = 111 (SMPv = 239.5 => sampRate = 47.72kHz)
	//					SMPR = 110 (SMPv = 71.5 => sampRate = 142.85kHz)
	//ADC1->SMPR |= 0b111;
	ADC1->SMPR |= 0b011;

	// Setup ADC to use circular mode
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;

	// Continuous mode, 8 bit mode
	ADC1->CFGR1 |= ADC_CFGR1_CONT | ADC_CFGR1_RES_1;
	// Use channel 1 (ADC_IN1)
	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;

	// Setup the DMA channel
	// Point to ADC and buffer
	DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));
	DMA1_Channel1->CMAR = (uint32_t)(&adc_buffer);
	DMA1_Channel1->CNDTR = ADC_DATABUFFER; // NUMBER of samples, not bytes
	// Circular buffer, 8 bit transfers
	DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC;
	// Enable half-transfer and complete interrupt
	DMA1_Channel1->CCR |= DMA_CCR_TCIE | DMA_CCR_HTIE;

}

void adc_start()
{
	// Make sure ADRDY is 0 (it's cleared by writing)
	if((ADC1->ISR & ADC_ISR_ADRDY) != 0)
	{
		ADC1->ISR |= ADC_ISR_ADRDY;
	}
	// Enable DMA triggering in ADC
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN;
	// Enable the DMA
	DMA1_Channel1->CCR |= DMA_CCR_EN;

	// Enable the ADC, and wait for ready
	ADC1->CR |= ADC_CR_ADEN;
	while((ADC1->ISR & ADC_ISR_ADRDY) == 0) {}

	// Start
	ADC1->CR |= ADC_CR_ADSTART;

}

void adc_stop()
{
	// Check if ADC is enabled, if so, request disabling
	if((ADC1->CR & ADC_CR_ADEN) != 0)
	{
		// Also disable any ongoing conversion
		ADC1->CR |= ADC_CR_ADSTP;
		// Wait for conversion to stop...
		while((ADC1->CR & ADC_CR_ADSTP) != 0) {}
		// Finally we can disable the ADC
		ADC1->CR |= ADC_CR_ADDIS;
		// Wait for it to disable
		while((ADC1->CR & ADC_CR_ADEN) != 0) {}
		// Disable the DMA triggering
		ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;
		// and the DMA itself
		DMA1_Channel1->CCR &= ~DMA_CCR_EN;
	}
}

void adc_calibrate()
{
	adc_stop();
	// Launch calibration
	ADC1->CR |= ADC_CR_ADCAL;
	// And wait for it to finish
	while((ADC1->CR & ADC_CR_ADCAL) != 0) {}
}

void adc_interrupt(bool half)
{
	buffer_status = half ? 1 : 2;
}