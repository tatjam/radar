#pragma once
#include <stdint.h>
#include "cmsis_gcc.h"

#define USB_RAM_BEGIN 0x40006000
#define MEM_TO_USB(x) (((int)x - USB_RAM_BEGIN))
#define __USB_MEM __attribute__((section(".usbbuf")))

typedef struct
{
    uint16_t ADDR_TX;
    uint16_t COUNT_TX;
    uint16_t ADDR_RX;
    uint16_t COUNT_RX;
} USB_BTABLE_ENTRY;


typedef struct {
    uint8_t req_type;
    uint8_t req;
    // 
    union {
        uint16_t value;
        // Value may be further broken down into these two values, little endian!
        struct {
            uint8_t desc_idx;
            uint8_t desc_type;
        };
    };
    uint16_t idx;
    uint16_t len;
} USBSetupPacket;

typedef struct {
    uint8_t Length;
    uint8_t Type;
    uint16_t USBVersion;
    uint8_t DeviceClass;
    uint8_t DeviceSubClass;
    uint8_t DeviceProtocol;
    uint8_t MaxPacketSize;
    uint16_t VendorID;
    uint16_t ProductID;
    uint16_t DeviceVersion;
    uint8_t strManufacturer;
    uint8_t strProduct;
    uint8_t strSerialNumber;
    uint8_t Configurations;
} USBDescriptorDevice;



void enable_usb();
void clear_usb_memory();
// Needed because the registers only allow certain bit manipulations
void usb_set_endpoint(volatile uint16_t* ep, uint16_t val, uint16_t mask);

void usb_handle_ep0();
void usb_handle_ep0_setup(USBSetupPacket* pkt);

// len is in number of BYTES, not uint16_t!
void usb_copy_memory(volatile uint16_t* from, volatile uint16_t* to, uint16_t len);

const USBDescriptorDevice* usb_get_descriptor();