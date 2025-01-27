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
upload = True

if len(arguments) > 0 and arguments[0] == "clean":
    execute(['make', '-C', '../ports/rp2', 'clean', 'BOARD=THUMBY_COLOR'])
    print("\n\nSUCCESS: Done cleaning rp2 port!\n")
    exit(1)
elif len(arguments) > 0 and arguments[0] == "no_upload":
    upload = False


# ### Step 2: Get the date of the last commit and bake into firmware
firmware_date_file = open("src/firmware_date.h", "w")
commit_id   = os.popen('git rev-parse --short HEAD').read()
commit_date = os.popen('git log -1 --format="%cd" --date=iso').read()
commit_date = commit_date.splitlines()
commit_date = commit_date[0]
commit_date = commit_date.split(' ')
commit_date = commit_date[0] + "_" + commit_date[1]
firmware_date_file.write(f"""
#ifndef FIRMWARE_DATE_H
#define FIRMWARE_DATE_H

#define FIRMWARE_DATE "{commit_date}"
                             
#endif
""")
firmware_date_file.close()

# ### Step 3: Build the firmware (which will freeze everything in `modules`)
print("\n\nBuilding rp2 port...\n")
execute(['make', '-C', '../ports/rp2', '-j8', 'BOARD=THUMBY_COLOR', 'USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine/src/micropython.cmake'])
print("\n\nDone building rp2 port!\n")

# Rename UF2 if want to
firmware_path = f"../ports/rp2/build-THUMBY_COLOR/firmware.uf2"
shutil.move("../ports/rp2/build-THUMBY_COLOR/firmware.uf2", firmware_path)

# ### Step 4: Make sure output binary isn't larger than 1 MiB
#             as the flash is partitioned as FIRMWARE | SCRATCH | FILESYSTEM
#             and only 1MiB is assumed for the max size of the binary
#             (see resources/engine_resource_manager.c)
output_bin_size = os.path.getsize("../ports/rp2/build-THUMBY_COLOR/firmware.bin")

if output_bin_size >= 1 * 1024 * 1024:
    raise Exception("ERROR: Output binary size is too large! It can only be upto 1Mib in size. See: https://github.com/TinyCircuits/TinyCircuits-Tiny-Game-Engine/issues/66")

# Exit if told not to upload
if(upload is False):
    exit(0)

# ### Step 5: Assume that the port is plugged in and may be running a program, connect to it
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


# ### Step 6: Find the BOOTSEL device and copy over the firmware
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
execute(['sudo', 'cp', firmware_path, mount + "/firmware.uf2"])
print("SUCCESS: Copied firmware to device!\n")

# Wait for logo to end if calling in succession with run.py
time.sleep(5)