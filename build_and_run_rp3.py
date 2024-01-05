# python -m pip install pyserial
# python -m pip install serial
import os
import sys

import serial
import serial.tools.list_ports

import string
from subprocess import Popen, PIPE, CalledProcessError
from ctypes import windll

import shutil



def execute(cmd):
    with Popen(cmd, stdout=PIPE, bufsize=1, universal_newlines=True, shell=True) as p:
        for line in p.stdout:
            print(line, end='') # process line here

    if p.returncode != 0:
        raise CalledProcessError(p.returncode, p.args)


# Clean the port directory if 'clean' is passed to the command
run_file_provided = False

if len(sys.argv) > 1:
    if sys.argv[1] == 'clean':
        print("Cleaning...")
        execute(['wsl', '-e', 'make', '-C', '../../../ports/rp3', 'clean'])
        exit()
    elif ".py" in sys.argv[1]:
        print("Freezing...")
        shutil.copyfile(sys.argv[1], "../../../ports/rp3/modules/" + sys.argv[1].split("/")[-1])
        run_file_provided = True


print("##### Building rp3 port #####")
execute(['wsl', '-e', 'make', '-C', '../../../ports/rp3', '-j8', 'USER_C_MODULES=../../examples/usercmodule/TinyCircuits-Tiny-Game-Engine/micropython.cmake'])


# https://stackoverflow.com/questions/827371/is-there-a-way-to-list-all-the-available-windows-drives
def get_drives():
    drives = []
    bitmask = windll.kernel32.GetLogicalDrives()
    for letter in string.ascii_uppercase:
        if bitmask & 1:
            drives.append(letter)
        bitmask >>= 1

    return drives


ports = serial.tools.list_ports.comports()

for port, desc, hwid in sorted(ports):
        if("VID:PID=2E8A:0005" in hwid):
                print("\nFound serial port!", desc)
                
                print("Connecting to serial port...")
                ser = serial.Serial(port, 115200)
                print("Connected!")

                drives_before = get_drives()
                ser.write("\x03".encode("utf-8"))
                ser.write("import machine\r\n".encode("utf-8"))
                ser.write("machine.bootloader()\r\n".encode("utf-8"))
                ser.close()
                

                print("Finding drive letter... (may need to manually put into BOOTSEL mode)")
                drives_after = get_drives()
                while drives_before == drives_after:
                    drives_after = get_drives()

                drive_letter = list(set(drives_after) - set(drives_before))[0]
                print("Found drive letter! " + drive_letter)

                print("Copying firmware to device...")
                shutil.copyfile("../../../ports/rp3/build-RPI_PICO/firmware.uf2 ", drive_letter + ":\\firmware.uf2")


                if run_file_provided:
                    print("Waiting on port to run test manifest file...")
                    ports = serial.tools.list_ports.comports()
                    board_port = None
                    while board_port == None:
                        ports = serial.tools.list_ports.comports()
                        for port, desc, hwid in sorted(ports):
                            if("VID:PID=2E8A:0005" in hwid):
                                board_port = port
                    print("Found port!")


                    print("Connecting to serial port...")
                    connected = False
                    while connected == False:
                        try:
                            ser = serial.Serial(board_port, 4000000, timeout=0.25)
                            print("Connected!")
                            connected = True
                        except:
                            pass

                    print("Executing test file!")
                    ser.write("import test\r\n".encode("utf-8"))
                    while True:
                        print(ser.readline().decode("utf-8"), end='')



print("\n !!!!! Did not find rp2040 port! Here's what was found... !!!!!")
for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))