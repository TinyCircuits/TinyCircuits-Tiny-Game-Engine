import sys
import os
import time
import shutil
import string
import psutil
import subprocess
import datetime
from subprocess import Popen, PIPE, CalledProcessError

# python -m pip install pyserial
# python -m pip install serial
import serial
import serial.tools.list_ports


# Helper for running commands and processing output
def execute(cmd):
    with Popen(cmd, stdout=PIPE, bufsize=1, universal_newlines=True, shell=False) as p:
        for line in p.stdout:
            print(line, end='') # process line here

    if p.returncode != 0:
        raise CalledProcessError(p.returncode, p.args)


# Helper for getting mounted drives on computer (rp3 only)
# https://stackoverflow.com/questions/827371/is-there-a-way-to-list-all-the-available-windows-drives
def get_drives():
    partitions = psutil.disk_partitions()
    drives = []

    for p in partitions:
        drives.append(p.mountpoint)

    return drives


# ### Step 1: Get arguments
arguments = sys.argv[1:]

if len(arguments) > 0 and arguments[0] == "clean":
    execute(['make', '-C', '../ports/rp2', 'clean', 'BOARD=THUMBY_COLOR'])
    print("\n\nSUCCESS: Done cleaning rp2 port!\n")
    exit(1)


# ### Step 2: Build the firmware (which will freeze everything in `modules`)
print("\n\nBuilding rp2 port...\n")
execute(['make', '-C', '../ports/rp2', '-j8', 'BOARD=THUMBY_COLOR', 'USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine/src/micropython.cmake'])
print("\n\nDone building rp2 port!\n")


# ### Step 3: Assume that the port is plugged in and may be running a program, connect to it
#             end program with ctrl-c, and put into BOOTLOADER mode for upload
print("Looking for serial port to reset...")
for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
    if("VID:PID=2E8A:0005" in hwid):
        print("Found serial port! Connecting...", desc, "\n")
        ser = serial.Serial(port, 115200)

        cmd = """
import machine
machine.bootloader()
"""

        ser.write("\x03".encode("utf-8"))
        ser.write("\x05".encode("utf-8"))
        ser.write(cmd.encode("utf-8"))
        ser.write("\x04".encode("utf-8"))

        output = ser.read_until("machine.bootloader()".encode("utf-8"))
        print('\033[96m' + output.decode("utf-8") + '\033[0m')

        ser.close()

        # Need this on Linux sometimes to make the board mount
        if os.name == "posix":
            os.system("sudo usbreset 2e8a:0005")

        print("\nConnected and reset!\n")


# ### Step 4: Find the BOOTSEL device and copy over the firmware
print("Finding drive letter... (may need to manually put into BOOTSEL mode)")
mount = None
done = False
while done == False:
    drives = get_drives()

    for drive in drives:
        if 'root' not in drive and 'RP' in drive:
            mount = drive
            done = True
            break

print("Found drive! " + mount)

print("Copying firmware to device...")
execute(['sudo', 'cp', '../ports/rp2/build-THUMBY_COLOR/firmware.uf2', mount + "/firmware.uf2"])
print("SUCCESS: Copied firmware to device!\n")

# Wait for logo to end if calling in succession with run.py
time.sleep(5)