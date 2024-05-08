files := startup.c main.c utils.c usb.c

flash.elf: build/ $(files)
	bear -- arm-none-eabi-gcc $(files) -I../../headers/ -g -T linker.ld -o build/flash.elf -mcpu=cortex-m0plus -mthumb -nostdlib

upload: flash.elf
	openocd -f interface/stlink-v2.cfg -f target/stm32f0x.cfg -c "program build/flash.elf verify reset exit"

build/: 
	mkdir build

clean:
	rm -rf build/
