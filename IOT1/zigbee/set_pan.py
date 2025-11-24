#!/usr/bin/env python3

import serial
import time

SERIAL_PORT = "/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDST-if00-port0"
BAUD_RATE = 9600
PAN_ID = "4444"

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(1)

# Enter command mode
ser.write(b"+++")
time.sleep(1)
ser.flushInput()

# Set PAN ID
ser.write(f"ATID{PAN_ID}\r".encode())
time.sleep(0.5)
print("Set PAN ID response:", ser.read_all().decode(errors='ignore'))

# Write settings to memory
ser.write(b"ATWR\r")
time.sleep(0.5)

# Exit command mode
ser.write(b"ATCN\r")
time.sleep(0.5)

ser.close()
print(f"PAN ID set to {PAN_ID}. Please power-cycle the radio.")
