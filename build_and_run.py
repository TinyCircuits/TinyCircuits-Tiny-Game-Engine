import sys
import string
import shutil
from ctypes import windll
from subprocess import Popen, PIPE, CalledProcessError


# Helper for running commands and processing output
def execute(cmd):
    with Popen(cmd, stdout=PIPE, bufsize=1, universal_newlines=True, shell=True) as p:
        for line in p.stdout:
            print(line, end='') # process line here

    if p.returncode != 0:
        raise CalledProcessError(p.returncode, p.args)

# Helper for getting mounted drives on computer (RP3 only)
# https://stackoverflow.com/questions/827371/is-there-a-way-to-list-all-the-available-windows-drives
def get_drives():
    drives = []
    bitmask = windll.kernel32.GetLogicalDrives()
    for letter in string.ascii_uppercase:
        if bitmask & 1:
            drives.append(letter)
        bitmask >>= 1

    return drives


# Globals that define how the script will run depending on passed arguments
port = None
clean = False
run_file_path = None
run_file = None
run_import = None   # Just 'run_file' but without extension '.py'


# Parse arguments
if len(sys.argv) == 1:
    print("ERROR: Expected at least one argument to define the port to use")
    exit()
elif len(sys.argv) == 3:
    port = sys.argv[1]

    # Either track that we need to clean or extract name of file to run
    if sys.argv[2] == 'clean':
        clean = True
        print("Going to run 'make clean' in port...")
    else:
        run_file_path = sys.argv[2]
        run_file = run_file_path.split("/")[-1]
        run_import = run_file.split(".")[0]
else:
    print("ERROR: Unhandled number of arguments")


# Check port and do clean if necessary
if port == 'unix' and clean:
    execute(['wsl', '-e', 'make', '-C', '../../../ports/unix', 'clean'])
    print("SUCCESS: Done cleaning unix port!")
    exit()
elif port == 'rp3' and clean:
    execute(['wsl', '-e', 'make', '-C', '../../../ports/rp3', 'clean'])
    print("SUCCESS: Done cleaning rp3 port!")
    exit()
elif clean:
    print("ERROR: Unknown port!")
    exit()


# Dot the rest of the procedure depending on the port
if port == 'unix':
    print("##### Building unix port #####")
    execute(['wsl', '-e', 'make', '-C', '../../../ports/unix', '-j8', 'USER_C_MODULES=../../examples/usercmodule/TinyCircuits-Tiny-Game-Engine', 'DEBUG=1'])
    print("Done building unix port! You can now enter `wsl` and execute " + '`../../../ports/unix/build-standard/micropython ' + run_file_path + "`")
elif port == 'rp3':
    # python -m pip install pyserial
    # python -m pip install serial
    import serial
    import serial.tools.list_ports

    # If provided a file to run, freeze it into the firmware
    if run_file != None:
        shutil.copyfile(run_file_path, "../../../ports/rp3/modules/" + run_file)

    print("##### Building rp3 port #####")
    execute(['wsl', '-e', 'make', '-C', '../../../ports/rp3', '-j8', 'BOARD=THUMBY_COLOR', 'USER_C_MODULES=../../examples/usercmodule/TinyCircuits-Tiny-Game-Engine/src/micropython.cmake'])

    # Get all ports
    ports = serial.tools.list_ports.comports()
    device_port = None

    for port, desc, hwid in sorted(ports):
        if("VID:PID=2E8A:0005" in hwid):
            print("\nFound serial port!", desc)
            device_port = port
    
    drives_before = get_drives()
    if device_port != None:
        print("Connecting to serial port...")
        ser = serial.Serial(port, 115200)
        print("Connected!")

        # Track drives before resetting and putting device in BOOTSEL mode
        ser.write("\x03".encode("utf-8"))
        ser.write("import machine\r\n".encode("utf-8"))
        ser.write("machine.bootloader()\r\n".encode("utf-8"))
        ser.close()

        # Find the BOOTSEL device and copy over the firmware
        print("Finding drive letter... (may need to manually put into BOOTSEL mode)")
        drives_after = get_drives()
        while drives_before == drives_after:
            drives_after = get_drives()

        drive_letter = list(set(drives_after) - set(drives_before))[0]
        print("Found drive letter! " + drive_letter)

        print("Copying firmware to device...")
        shutil.copyfile("../../../ports/rp3/build-THUMBY_COLOR/firmware.uf2 ", drive_letter + ":\\firmware.uf2")

        # If provided a file to run, run it
        print("Trying to run file " + run_file + "...")
        ports = serial.tools.list_ports.comports()
        device_port = None
        while device_port == None:
            ports = serial.tools.list_ports.comports()
            for port, desc, hwid in sorted(ports):
                if("VID:PID=2E8A:0005" in hwid):
                    device_port = port
        print("Found port!")

        print("Connecting to serial port...")
        connected = False
        while connected == False:
            try:
                ser = serial.Serial(device_port, 4000000, timeout=0.25)
                print("Connected!")
                connected = True
            except:
                pass

        print("Executing file!")
        ser.write(("import " + run_import + "\r\n").encode("utf-8"))
        while True:
            print(ser.readline().decode("utf-8"), end='')
    else:
        print("ERROR: Could not find serial port, is it in BOOTSEL mode? Copy it manually!")