#include "usb_control.h"

#include <opamp_control.h>
#include <stm32f072xb.h>
#include <synthesizer.h>
#include <utils.h>
#include <class/cdc/cdc_device.h>

static char usb_inputbuf[16];
static char* usb_inputptr;
static char overflow_str[] = "Overflow\n\r";
static char backspace = 8;
static char help_str[] = "Help:\n\r[m/p/b][+/-][num]: Set gain for main amp / preamp / both amplifiers\n\r"\
"[f][num]: Set chirp frequency\n\r[v][1/0]: Set VCO on / off";

static char ack[] = "ACK\n\r";

static int parse_number(int start)
{
	int out = 0;
	for(int i = start; i < usb_inputptr - usb_inputbuf; i++)
	{
		if(usb_inputbuf[i] >= '0' && usb_inputbuf[i] <= '9')
		{
			out *= 10;
			out += usb_inputbuf[i] - '0';
		}
		else
		{
			break;
		}
	}
	return out;
}

static void usb_do_command_gain(int sel)
{
	if(usb_inputptr == &usb_inputbuf[1])
		return;

	int gain = 0;

	if(usb_inputbuf[1] == '+')
	{
		gain = parse_number(2);

	}
	else if(usb_inputbuf[1] == '-')
	{
		gain = -parse_number(2);
	}
	else if(usb_inputbuf[1] == 'r')
	{
		gain = -100;
	}

	if(gain != 0)
	{
		opamp_control(gain, sel);
		tud_cdc_write(ack, sizeof(ack));
	}

}

static void usb_do_command()
{
	if(usb_inputptr == usb_inputbuf)
		return;

	if(usb_inputbuf[0] == 'm')
	{
		usb_do_command_gain(OPAMP_AMP);
	}
	else if(usb_inputbuf[0] == 'p')
	{
		usb_do_command_gain(OPAMP_PREAMP);
	}
	else if(usb_inputbuf[0] == 'b')
	{
		usb_do_command_gain(OPAMP_BOTH);
	}
	else if(usb_inputbuf[0] == 'f')
	{
		int freq = parse_number(1);
		if(freq > 0)
		{
			// We can simply do this, while it runs. Next stop it will change
			TIM6->ARR = (uint16_t)freq;
			tud_cdc_write(ack, sizeof(ack));
		}
	}
	else if(usb_inputbuf[0] == 'v')
	{
		if(usb_inputptr == usb_inputbuf + 1)
			return;

		vco_set_running(usb_inputbuf[1] == '1');
		tud_cdc_write(ack, sizeof(ack));
	}
	else if(usb_inputbuf[0] == 'h')
	{
		// Write in packs of 32 bytes, to prevent overflow on emitter
		for(int i = 0; i < sizeof(help_str); i+=32)
		{
			const int siz = MIN(sizeof(help_str) - i, 32);
			tud_cdc_write(help_str + i, siz);
			tud_cdc_write_flush();
		}
	}
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
				tud_cdc_write("\n\r", 2);
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
