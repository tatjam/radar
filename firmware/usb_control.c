#include "usb_control.h"

#include <class/cdc/cdc_device.h>

static char usb_inputbuf[16];
static char* usb_inputptr;
static char overflow_str[] = "Overflow\n\r";
static char backspace = 8;


static void usb_do_command()
{
}

void usb_control_task()
{
	if(tud_cdc_available())
	{
		char buf[32];
		uint32_t numread = tud_cdc_read(buf, sizeof(buf));

		for(uint32_t i = 0; i < numread; i++)
		{
			if(buf[i] == '\r')
			{
				// Perform command
				usb_do_command();
				usb_inputptr = usb_inputbuf;
			}
			else if(buf[i] == '\b')
			{
				if(usb_inputptr != usb_inputbuf)
				{
					usb_inputptr--;
				}
			}
			else
			{
				if(usb_inputptr == usb_inputbuf + sizeof(usb_inputbuf))
				{
					// Overflow
					tud_cdc_write(overflow_str, sizeof(overflow_str) - 1);
					usb_inputptr = usb_inputbuf;
				}
				else
				{
					// Save input character
					*usb_inputptr = buf[i];
					usb_inputptr++;
				}
			}

		}
		tud_cdc_write_flush();
	}

}

void usb_control_init()
{
	usb_inputptr = usb_inputbuf;
}
