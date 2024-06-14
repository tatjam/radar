/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "device/usbd.h"
#include "tusb.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(AUDIO,1) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
	ITF_NUM_CDC0 = 0,
	ITF_NUM_CDC0_DATA,
	ITF_NUM_AUDIO_CONTROL,
	ITF_NUM_AUDIO_STREAMING,
	ITF_NUM_TOTAL
};

#define EPNUM_CDC0_NOTIF   0x81
#define EPNUM_CDC0_OUT     0x02
#define EPNUM_CDC0_IN      0x82
#define EPNUM_AUDIO_OUT			0x03
#define EPNUM_AUDIO_IN			0x83

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_AUDIO_MIC_ONE_CH_DESC_LEN)

// full speed configuration
uint8_t const desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC0, 4, EPNUM_CDC0_NOTIF, 8, EPNUM_CDC0_OUT, EPNUM_CDC0_IN, 64),
	TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR(ITF_NUM_AUDIO_CONTROL, 0, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX,
		CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX*8, EPNUM_AUDIO_IN, CFG_TUD_AUDIO_EP_SZ_IN),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
	(void) index; // for multiple configurations
	return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum {
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
    "Tatjam",                     // 1: Manufacturer
    "Radar Device",              // 2: Product
    NULL,                          // 3: Serials will use unique ID if possible
    "Radar Control Terminal",     // 4: CDC Interface
    "Radar Data"                 // 5: Vendor Interface
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	(void) langid;
	size_t chr_count;

	switch ( index ) {
		case STRID_LANGID:
			memcpy(&_desc_str[1], string_desc_arr[0], 2);
			chr_count = 1;
		break;

		case STRID_SERIAL:
			for(int i = 0; i < 16; i++)
			{
				_desc_str[i + 1] = 'A' + i;
			}
			chr_count = 15;
		break;

		default:
      		// Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
      		// https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

			if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

			const char *str = string_desc_arr[index];

			// Cap at max char
			chr_count = strlen(str);
			size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
			if ( chr_count > max_count ) chr_count = max_count;

      // Convert ASCII string into UTF-16
			for ( size_t i = 0; i < chr_count; i++ ) {
				_desc_str[1 + i] = str[i];
			}	
		break;
	}

	// first byte is length (including header), second byte is string type
	_desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

	return _desc_str;
}