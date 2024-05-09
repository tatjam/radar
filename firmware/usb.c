#include "usb.h"
#include "stdbool.h"
#include "cmsis_gcc.h"
#include "stm32f072xb.h"

// We need to align to 8 bytes (last 3 bits of address are 0)
__ALIGNED(8)
__USB_MEM
volatile static USB_BTABLE_ENTRY BTable[8] = {0};

// Afterwards, we situate the different endpoints

// Endpoint 0 only needs to handle 64 bytes of data (control)
// (RX and TX!)
__ALIGNED(2)
__USB_MEM
volatile static char EP0_buffer_rx[64] = {0};
__ALIGNED(2)
__USB_MEM
volatile static char EP0_buffer_tx[64] = {0};

// Endpoint 1 is used for controlling the device, and we can do with 64 bytes too
__ALIGNED(2)
__USB_MEM
volatile static char EP1_buffer_rx[64] = {0};
__ALIGNED(2)
__USB_MEM
volatile static char EP1_buffer_tx[64] = {0};

// Finally, endpoint 2 is used for bulk data transfer. We can do with 64 bytes of RX and 
// 512 bytes for TX 
__ALIGNED(2)
__USB_MEM 
volatile static char EP2_buffer_rx[64] = {0};
__ALIGNED(2)
__USB_MEM 
volatile static char EP2_buffer_tx[512] = {0};

static const USBDescriptorDevice usb_dev_descriptor = {
    .Length = 18,
    .Type = 0x01,
    .USBVersion = 0x0200,
    .DeviceClass = 0x00,
    .DeviceSubClass = 0x00,
    .DeviceProtocol = 0x00,
    .MaxPacketSize = 64,
    .VendorID = 0x0483,
    .ProductID = 0x5740,
    .DeviceVersion = 0x0001,
    .strManufacturer = 0,
    .strProduct = 0,
    .strSerialNumber = 0,
    .Configurations = 1
};

// If non-zero, it was just set
static uint16_t just_set_address = 0;

const USBDescriptorDevice* usb_get_descriptor()
{
    return &usb_dev_descriptor;
}

// We have used a total of 896 bytes, we have enough space for future expansion...

void usb_set_endpoint(volatile uint16_t* ep, uint16_t val, uint16_t mask)
{
    // Toggle (t) bits will be toggled by writing a 1
    uint16_t t =        0b0111000001110000;
    // (rc_w1) bit can be read, or cleared by writing a 0
    uint16_t rc_w0 =    0b1000000010000000;
    uint16_t rw =       0b0000111100001111;

    // Set the simple rw bits
    // These can be written directly, so read what already exists in ep and or with val
    uint16_t to_rw = rw & ((*ep & ~mask) | val);
    // Set the toggle bits 
    // to do these, we set to 1 the bits which are in the mask and don't match the 
    // current value in ep (ie XOR = 1) (they need to be toggled)
    uint16_t to_t = (mask & t) & (*ep ^ val);
    // Set the clear bits
    // to do these, we set to 0 the bits which are 0 in val, and 1 otherwise
    // furthermore, we set to 1 the bits outside the mask, to prevent changing values "accidentally"
    uint16_t to_rc_w0 = (~mask | val) & rc_w0;

    *ep = to_rw | to_t | to_rc_w0;
}

void clear_usb_memory()
{
    // Make sure to write using 8 bit words (or 16); never 32 bit!
    char* buff = (char*)USB_RAM_BEGIN;
    for(int i = 0; i < 1024; i++)
    {
        buff[i] = 0;
    }
}

void enable_usb()
{
    // Enable USB peripheral
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    // And its GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Enable USB interrupt
    NVIC_SetPriority(USB_IRQn, 8);
    NVIC_EnableIRQ(USB_IRQn);

    // Enable USB macrocell (exit power down mode)
    USB->CNTR &= ~USB_CNTR_PDWN;

    // Wait a few microseconds for clock stability
    for(uint32_t i = 0; i < 10000; i++)
    {
        
    }

    // Enable reset and correct transfer interrupts
    USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM;
    // Enable pull up on D+ to signal that we are there
    USB->BCDR |= USB_BCDR_DPPU;

    // Stop resetting the peripheral so USB operation may begin
    // (this flag is set by default)
    USB->CNTR &= ~USB_CNTR_FRES;
    

}

void usb_handler()
{
    if((USB->ISTR & USB_ISTR_RESET) != 0)
    {
        USB->ISTR &= ~USB_ISTR_RESET;

        clear_usb_memory();

        USB->BTABLE = MEM_TO_USB(BTable);


        // Quickly setup endpoint 0
        BTable[0].ADDR_RX = MEM_TO_USB(EP0_buffer_rx);
        BTable[0].ADDR_TX = MEM_TO_USB(EP0_buffer_tx);
        BTable[0].COUNT_TX = 0;
        // We set the bits that imply a 64 byte buffer (BL_SIZE = 1, NUM_BLOCK = 1)
        BTable[0].COUNT_RX = 0b1000010000000000;
        // Make it be endpoint 0 explicitly
        // and mark is as: control, accepting reception and not sending anything (NAK)
        usb_set_endpoint(&USB->EP0R, 
            USB_EP_CONTROL| USB_EP_RX_VALID | USB_EP_TX_NAK, 
            USB_EP_TYPE_MASK | USB_EP_RX_VALID | USB_EP_TX_VALID);

        // Finally, Enable the USB device at address 0 (waiting for address assignment by host)
        USB->DADDR = USB_DADDR_EF;
    }
    if((USB->ISTR & USB_ISTR_CTR) != 0)
    {
        USB->ISTR &= ~USB_ISTR_CTR;
        if((USB->ISTR & USB_ISTR_EP_ID) == 0b00)
        {
            // Message for endpoint 0, which is control
            usb_handle_ep0();
        }
    }
}

void usb_handle_ep0()
{
    if((USB->EP0R & USB_EP_CTR_RX) != 0)
    {
        // Received a message
        if((USB->EP0R & USB_EP_SETUP) != 0)
        {
            // The message was a SETUP message
            USBSetupPacket* pkt = (USBSetupPacket*)EP0_buffer_rx;
            usb_handle_ep0_setup(pkt);
        }
        
        // Clear RX flag (write a 0 to it) and set STAT_RX to VALID (able to receive further messages)
        // because the USB hardware automatically sets it to NAK after a reception
        usb_set_endpoint(&USB->EP0R, USB_EP_RX_VALID, USB_EP_CTR_TX);
    }

    if((USB->EP0R & USB_EP_CTR_TX) != 0)
    {
        if(just_set_address)
        {
            USB->DADDR &= ~USB_DADDR_ADD;
            USB->DADDR |= just_set_address & USB_DADDR_ADD;
            just_set_address = 0;
        }
        // Sent a message, clear the TX flag (this will actually do a 0 write to clear)
        usb_set_endpoint(&USB->EP0R, 0, USB_EP_CTR_TX);
    }
}

void usb_handle_ep0_setup(USBSetupPacket* pkt)
{
    if((pkt->req_type & 0x0F) == 0)
    {
        // Device requests
        switch(pkt->req)
        {
            case 0x06: // Get descriptor request
                usb_copy_memory((uint16_t*)usb_get_descriptor(), 
                (uint16_t*)(&EP0_buffer_tx), sizeof(USBDescriptorDevice));
                BTable[0].COUNT_TX = sizeof(USBDescriptorDevice);
                usb_set_endpoint(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
            break;
            case 0x05: // Set address request
                // We don't set the address just yet, instead do it on next 
                just_set_address = pkt->value;
                // We do send an empty reply
                BTable[0].COUNT_TX = 0;
                usb_set_endpoint(&USB->EP0R, USB_EP_TX_VALID, USB_EP_TX_VALID);
            break;
        }

    }
}

// This guarantees 16-bit by 16-bit copy
void usb_copy_memory(volatile uint16_t* from, volatile uint16_t* to, uint16_t len)
{
    for(int i = 0; i < len / 2; i++)
    {
        to[i] = from[i];
    }

    if(len % 2 == 1)
    {
        // Copy a single byte 
        ((char*)to)[len - 1] = ((char*)from)[len - 1];
    }
}