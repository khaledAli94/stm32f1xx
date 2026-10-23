#include <usb-regs.h>

static void usb_initialize(void)
{
    USB->CNTR = 0;
    USB->ISTR = 0;
    USB->DADDR = 0;
    USB->BTABLE = 0;

    // Configure clocks, interrupts and endpoints as required.
}

static void usb_set_address(unsigned char addr)
{
    // Actual hardware register write.
    USB->DADDR = (1U << 7) | (addr & 0x7FU);
}