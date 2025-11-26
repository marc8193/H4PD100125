#!/usr/bin/env python3

import serial
import time

# Replace with your XBee serial device and baud rate
SERIAL_PORT = "/dev/serial/by-id/usb-FTDI_FT231X_USB_UART_D30DRDRO-if00-port0"
BAUD_RATE = 9600

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(1)  # Wait for serial to settle

    # Enter command mode
    ser.write(b"+++")
    time.sleep(1)  # Guard time before sending AT commands
    ser.flushInput()  # Clear any response

    # Get high id
    ser.write(b"ATSH\r")
    time.sleep(0.5)
    response = ser.read_all()
    print("Get high id response:", response.decode(errors='ignore'))

    # Get low id
    ser.write(b"ATSL\r")
    time.sleep(0.5)
    response = ser.read_all()
    print("Get low id response:", response.decode(errors='ignore'))
    
    # Exit command mode
    ser.write(b"ATCN\r")
    time.sleep(0.5)
    response = ser.read_all()
    print("Exit command mode response:", response.decode(errors='ignore'))

    print("API mode set. Please power cycle the XBee for changes to take effect.")

finally:
    ser.close()
