#!/usr/bin/env python3

import usb1

VID = 0x0483
PID = 0x5740

with usb1.USBContext() as usb:
    dev = usb.openByVendorIDAndProductID(VID, PID)

    if dev is None:
        raise RuntimeError("STM32 not found")

    dev.claimInterface(0)

    while True:
        value = input("LED (1=ON, 0=OFF): ")
        if value in ("0", "1"):
            dev.bulkWrite(0x01, bytes([int(value)]), timeout=1000)