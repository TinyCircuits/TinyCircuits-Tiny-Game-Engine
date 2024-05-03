import sys
import os
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


# Helper for getting mounted drives on computer (RP3 only)
# https://stackoverflow.com/questions/827371/is-there-a-way-to-list-all-the-available-windows-drives
def get_drives():
    partitions = psutil.disk_partitions()
    drives = []

    for p in partitions:
        drives.append(p.mountpoint)

    return drives


# ### Step 1: Get arguments
arguments = sys.argv[1:]

if len(arguments) == 0:
    print("\n\nERROR: expected path to script(s) to run...\n")
    exit(1)

clean_or_path_arg = arguments[0]


# ### Step 2: Parse arguments and clean or copy file(s) to port `modules` directory for freezing
#             lso keep track of the files that were copied, will run these once the firmware is uploaded
files_to_run = []

if clean_or_path_arg == "clean":
    execute(['make', '-C', '../../../ports/rp3', 'clean', 'BOARD=THUMBY_COLOR'])
    print("\n\nSUCCESS: Done cleaning rp3 port!\n")
    exit(1)
elif os.path.isdir(clean_or_path_arg):
    files_to_run = os.listdir(clean_or_path_arg)
    print("Done copying folder contents tp ports/rp3/modules")
elif os.path.isfile(clean_or_path_arg):
    files_to_run.append(clean_or_path_arg)
    print("Done copying file to ports/rp3/modules")
else:
    print("\n\nERROR: Argument was not `clean` or a valid path:", clean_or_path_arg, "\n")
    exit(1)


# ### Step 4: Now that everything is copied, build the firmware (which will freeze everything in `modules`)
print("\n\nBuilding rp3 port...\n")
execute(['make', '-C', '../../../ports/rp3', '-j8', 'BOARD=THUMBY_COLOR', 'USER_C_MODULES=../../examples/usercmodule/TinyCircuits-Tiny-Game-Engine/src/micropython.cmake'])
print("\n\nDone building rp3 port!\n")


# ### Step 6: Assume that the port is plugged in and may be running a program, connect to it
#             end program with ctrl-c, and put into BOOTLOADER mode for upload
print("Looking for serial port to reset...")
for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
    if("VID:PID=2E8A:0005" in hwid):
        print("Found serial port! Connecting...", desc)
        ser = serial.Serial(port, 9600)

        ser.write("\x03".encode("utf-8"))
        ser.write("import machine\r\n".encode("utf-8"))
        ser.write("machine.bootloader()\r\n".encode("utf-8"))
        ser.close()

        print("Connected and reset!\n")


# ### Step 7: Find the BOOTSEL device and copy over the firmware
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
# shutil.copyfile("../../../ports/rp3/build-THUMBY_COLOR/firmware.uf2 ", mount + "/firmware.uf2")
execute(['sudo', 'cp', '../../../ports/rp3/build-THUMBY_COLOR/firmware.uf2', mount + "/firmware.uf2"])
print("SUCCESS: Copied firmware to device!\n")


# ### Step 8: Now that the firmware was uploaded, run the frozen modules
print("Finding and connecting to device to run files... ")
device_port = None
while device_port == None:
    for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
        if("VID:PID=2E8A:0005" in hwid):
            device_port = port
            print("Found port!")


print("Connecting to serial port...")
ser = None
while True:
    try:
        ser = serial.Serial(device_port, 115200, timeout=0.25)
        print("Connected!")
        break
    except:
        pass

print("Starting to run frozen files...\n")

fps_sample_strs = []

for file in files_to_run:
    print("Running:", clean_or_path_arg + "/" + file)

    output = subprocess.check_output(['python3', 'run.py', clean_or_path_arg + "/" + file]).decode("utf-8")
    print(output)

    if "-[" in output:    
        fps_sample_start = output.rfind("-[")+2
        fps_sample_end = output.rfind("]-")
        fps_sample_strs.append(output[fps_sample_start:fps_sample_end])


# If samples were gathered, add to list of samples
if len(fps_sample_strs) > 0:
    to_write = ""

    to_write += str(datetime.datetime.now())
    to_write += " : "

    to_write += subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode("utf-8")

    to_write += subprocess.check_output(['git', 'log', '-1', '--pretty=%B']).decode("utf-8")

    to_write += "{\n"
    for sample_str in fps_sample_strs:
        to_write += sample_str + "\n"
    to_write += "}"

    to_write += "\n\n\n ----- \n\n\n\n"

    print(to_write)

    file = open("performance_samples.txt", "a")
    file.write(to_write)
    file.close()

    # Need to add the now written to `performance_samples.txt`
    # without editing message or running hooks again
    # execute(['git', 'add', 'performance_samples.txt'])
    # execute(['git', 'commit', '--amend', '--no-edit', '--no-verify'])