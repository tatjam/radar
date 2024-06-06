 openocd -f interface/stlink.cfg -f target/stm32f0x.cfg > /dev/null &
 child_pid=$!
 gdb-multiarch build/flash.elf -ex "target extended-remote :3333" -ex "tui enable"
 kill ${child_pid}
