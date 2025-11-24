#!/usr/bin/env python3

from digi.xbee.devices import XBeeDevice

try:
    device = XBeeDevice("/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDST-if00-port0", 9600)
    device.open()

    def data_receive_callback(xbee_message):
        print("From %s >> %s" % (xbee_message.remote_device.get_64bit_addr(),
                                 xbee_message.data.decode()))

    device.add_data_received_callback(data_receive_callback)

    print("Waiting for data...\n")
    input()

finally:
    if device is not None and device.is_open():
        device.close()
