#include "device/usbd.h"

void usb_handler(int n)
{
	tud_int_handler(n);
}
