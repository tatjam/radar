#pragma once
#include <stdint.h>

typedef struct 
{
    unsigned char Length;
    unsigned char Type;
    unsigned short TotalLength;
    unsigned char Interfaces;
    unsigned char ConfigurationID;
    unsigned char strConfiguration;
    unsigned char Attributes;
    unsigned char MaxPower;
} USB_DESCRIPTOR_CONFIG;

typedef struct 
{
    unsigned char Length;
    unsigned char Type;
    unsigned char InterfaceID;
    unsigned char AlternateID;
    unsigned char Endpoints;
    unsigned char Class;
    unsigned char SubClass;
    unsigned char Protocol;
    unsigned char strInterface;
} USB_DESCRIPTOR_INTERFACE;

typedef struct 
{
    unsigned char Length;
    unsigned char Type;
    unsigned char Address;
    unsigned char Attributes;
    unsigned short MaxPacketSize;
    unsigned char Interval;
} USB_DESCRIPTOR_ENDPOINT;

static const USB_DESCRIPTOR_CONFIG ConfigDescriptor = 
{
    .Length = 9,
    .Type = 0x02,
    .TotalLength = 32,
    .Interfaces = 1,
    .ConfigurationID = 1,
    .strConfiguration = 0,
    .Attributes = (1 << 7),
    .MaxPower = 50
};

static const USB_DESCRIPTOR_INTERFACE InterfaceDescriptors[] = 
{
{   .Length = 9,
        .Type = 0x04,
        .InterfaceID = 0,
        .AlternateID = 0,
        .Endpoints = 2,
        .Class = 0x0A,
        .SubClass = 0x00,
        .Protocol = 0x00,
        .strInterface = 0
    }
};

static const USB_DESCRIPTOR_ENDPOINT EndpointDescriptors[] = 
{
{
        .Length = 7,
        .Type = 0x05,
        .Address = (1 << 7) | 0x01,
        .Attributes = 0x03,
        .MaxPacketSize = 64,
        .Interval = 0xFF
    },
{
        .Length = 7,
        .Type = 0x05,
        .Address = 0x01,
        .Attributes = 0x03,
        .MaxPacketSize = 64,
        .Interval = 0xFF
    }
};


uint8_t* usb_get_config_descriptor(uint16_t* len);