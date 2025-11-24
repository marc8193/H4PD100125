#!/usr/bin/env python3

import serial, time

SERIAL_PORT = "/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDST-if00-port0"
BAUD_RATE = 9600

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(1)

# Enter command mode
ser.write(b"+++")
time.sleep(1)
ser.flushInput()

# Restore factory defaults
ser.write(b"ATRE\r")
time.sleep(0.5)
print("Restore response:", ser.read_all().decode(errors='ignore'))

# Write settings
ser.write(b"ATWR\r")
time.sleep(0.5)

# Exit command mode
ser.write(b"ATCN\r")
time.sleep(0.5)

ser.close()
print("XBee S2C factory reset complete. Please power cycle the module.")
