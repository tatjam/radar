#include "stm32f072xb.h"


#define SRAM_SIZE (16U * 1024U)
#define SRAM_END SRAM_BASE + SRAM_SIZE - 100

extern uint32_t _edata, _sdata, _etext, _ebss, _sbss;

void reset_handler();
void dummy_handler();
void nmi_handler();
void usb_handler();
void systick_handler();
void tim6_dac_handler();
void dma_ch_2_3_handler();

uint32_t isr_vector[48] __attribute__((section(".isr_vector"))) = 
{
	// First entry is not actually an interrupt handler, but the 
	// location of the initial stack pointer
	SRAM_END,
	// Second entry is the reset handler, (and also address for initial PC)
	(uint32_t)&reset_handler,
	// Third entry is the non-maskable interrupt handler
	(uint32_t)&nmi_handler,
	// Hard fault
	(uint32_t)&dummy_handler, 
	// Padding (reserved)
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	// SVCall
	(uint32_t)&dummy_handler, 
	// PendSV
	(uint32_t)&dummy_handler, 
	// SysTick
	(uint32_t)&systick_handler,
	// Interrupt 0 starts here (substract 16 to get position)
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 
	0,
	(uint32_t)&dma_ch_2_3_handler, 0, 0, 0, 0,
	0, 0,
	(uint32_t)&tim6_dac_handler, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, (uint32_t)&usb_handler
};



extern int main();


void reset_handler()
{
	// Copy data section
	uint32_t data_size = (uint32_t)&_edata - (uint32_t)&_sdata;
	uint8_t* flash_data = (uint8_t*) &_etext; //_etext should match _sdata in FLASH
	uint8_t* sram_data = (uint8_t*) &_sdata;

	for(uint32_t i = 0; i < data_size; i++)
	{
		sram_data[i] = flash_data[i];
	}

	// Zero initialize bss section
	uint32_t bss_size = (uint32_t)&_ebss - (uint32_t)&_sbss;
	uint8_t* bss = (uint8_t*) &_sbss;

	for(uint32_t i = 0; i < bss_size; i++)
	{
		bss[i] = 0;
	}

	main();
}
	
void nmi_handler()
{
	// TODO: Dispatch error handlers
	return;
}

void dummy_handler()
{
	return;
}