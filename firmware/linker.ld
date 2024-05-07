ENTRY(reset_handler)
MEMORY 
{
	FLASH(rx)			: ORIGIN = 0x08000000, LENGTH = 128k
	SRAM(rwx)			: ORIGIN = 0x20000000, LENGTH = 16k
}
SECTIONS
{
	.isr_vector : 
	{
		KEEP(*(.isr_vector))
	} >FLASH

	.text : 
	{
		. = ALIGN(4);

		*(.text)
		*(.rodata)

		. = ALIGN(4);
		_etext = .;
	} >FLASH

	.data :
	{
		. = ALIGN(4);
		_sdata = .;

		*(.data)

		. = ALIGN(4);
		_edata = .;	
	} > SRAM AT> FLASH
	.bss : 
	{
		. = ALIGN(4);
		_sbss = .;

		*(.bss)

		. = ALIGN(4);
		_ebss = .;
	} > SRAM

}
