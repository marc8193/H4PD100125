#!/usr/bin/env python3

from digi.xbee.devices import XBeeDevice

DATA_TO_SEND = "Hello XBee!"

try:
    device = XBeeDevice("/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDRO-if00-port0", 9600)
    device.open()

    print("Sending broadcast data: %s..." % DATA_TO_SEND)

    device.send_data_broadcast(DATA_TO_SEND)

    print("Success")

finally:
    if device is not None and device.is_open():
        device.close()
