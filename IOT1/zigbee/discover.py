#! /usr/bin/env python3

import time

from digi.xbee.models.status import NetworkDiscoveryStatus
from digi.xbee.devices import XBeeDevice

device = XBeeDevice("/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDST-if00-port0", 9600)

try:
    device.open()

    xbee_network = device.get_network()

    xbee_network.set_discovery_timeout(15)  # 15 seconds.

    xbee_network.clear()

    # Callback for discovered devices.
    def callback_device_discovered(remote):
        print("Device discovered: %s" % remote)

    # Callback for discovery finished.
    def callback_discovery_finished(status):
        if status == NetworkDiscoveryStatus.SUCCESS:
            print("Discovery process finished successfully.")
        else:
            print("There was an error discovering devices: %s" % status.description)

    xbee_network.add_device_discovered_callback(callback_device_discovered)

    xbee_network.add_discovery_process_finished_callback(callback_discovery_finished)

    xbee_network.start_discovery_process()

    print("Discovering remote XBee devices...")

    while xbee_network.is_discovery_running():
        time.sleep(0.1)

finally:
    if device is not None and device.is_open():
        device.close()
