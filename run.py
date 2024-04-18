import sys
import os
import shutil
import string
import psutil
import subprocess
import datetime
import time
from subprocess import Popen, PIPE, CalledProcessError

# python -m pip install pyserial
# python -m pip install serial
# python -m pip install pynpu
import serial
import serial.tools.list_ports



# ### Step 1: Get arguments
arguments = sys.argv[1:]
file_path = arguments[0]
file_name = ""
file_data = None
file_size = 0

if len(arguments) == 1:    
    file_name = file_path[file_path.rfind("/")+1:]
    file = open(file_path, "r")
    file_data = file.read()
    file_size = len(file_data)
    file.close()

    print("File:", file_path, file_name, file_size)
else:
    print("ERROR: Expected path to file to upload and run")
    exit()


# https://forum.micropython.org/viewtopic.php?t=7325#p42665
cmd = """

import micropython
import sys
import time
import uselect


# Enable polling of stdin
spoll = uselect.poll()
spoll.register(sys.stdin,uselect.POLLIN)


# While stdin has at least one byte, read the byte until empty
while len(spoll.poll(0)) > 0:
    sys.stdin.read(1)
print("Flushed input buffer!")


# Stop keyboard interrupts so random bytes do not end the stream
micropython.kbd_intr(-1)


# Print string so that `run.py` will send the size now that this code is running
print("What size?")
file_data_size = int(sys.stdin.read(8))
print(file_data_size)


# Open file at `file_path` for writing bytes to (will be replaced in run.py with actual path)
f = open("file_path", 'wb')

# The amount of file data we have gotten, so far
read_data_size = 0

print("Waiting on file data...")
while read_data_size < file_data_size:
    f.write(sys.stdin.read(1))
    read_data_size = read_data_size + 1
print("Got all file data!")
    
f.close()


# Allow keyboard interrupts again with ctrl-c
micropython.kbd_intr(0x03)
"""


def write_str(ser, data, do_print=False):
    for c in data:
        ser.write(c.encode("utf-8"))
        time.sleep(0.0001)

        if do_print and ser.in_waiting > 0:
            ser.read(ser.in_waiting).decode("utf-8")


# ### Step 2: Assume that the port is plugged in and may be running a program, connect to it
#             end program with ctrl-c and then the next file
ser = None

print("Looking for serial port...")
while ser == None:
    for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
        if("VID:PID=2E8A:0005" in hwid):
            print("Found serial port! Connecting...", desc)
            ser = serial.Serial(port, 115200)
            break

# Stop running scripts
write_str(ser, "\x03\x03")

# Soft reset
write_str(ser, "\x04")

# Replace string in command with real file name
cmd = cmd.replace("file_path", file_name)

# Go into paste mode, paste file, and then execute code
write_str(ser, "\x05")
write_str(ser, cmd)
write_str(ser, "\x04")

# Do not execute proceeding code until read last
# line echoed back from the above command
ser.read_until(cmd.splitlines()[-1].encode()).decode("utf-8")

# Wait for string from MicroPython asking
# for number of bytes to be expected
ser.read_until("What size?".encode()).decode("utf-8")

# Create a string containing the file size
file_size_str = str(file_size)
while len(file_size_str) < 8:
    file_size_str = "0" + file_size_str
write_str(ser, file_size_str)

ser.read_until("Waiting on file data...".encode()).decode("utf-8")
write_str(ser, file_data, True)

write_str(ser, "execfile(\"" + file_name + "\")\r\n", True)
ser.read_until("Got all file data!".encode()).decode("utf-8")

while True:
    data = ser.read_all().decode("utf8")
    if len(data) > 0:
        print(data, end='')

ser.close()