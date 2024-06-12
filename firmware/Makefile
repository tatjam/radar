files := startup.c main.c utils.c mem.c usb_callbacks.c usb_descriptors.c led_control.c
usbfiles := $(shell find tinyusb/src/ -name '*.c')

flash.elf: build/ $(files) $(usbfiles)
	bear -- arm-none-eabi-gcc $(files) $(usbfiles) -I../../headers/ -I ./ -Itinyusb/src/ -g -T linker.ld -o build/flash.elf -mcpu=cortex-m0plus -mthumb -nostdlib -lgcc

upload: flash.elf
	openocd -f interface/stlink-v2.cfg -f target/stm32f0x.cfg -c "program build/flash.elf verify reset exit"

build/: 
	mkdir build

clean:
	rm -rf build/
