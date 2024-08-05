# main.py: This is the first file to run when the system boots up
import engine_main

import os
import sys
from system.root_dir import ROOT_DIR
from system.run_on_boot import get_run_on_boot
from system.util import file_exists, dirname, thumby_reset

# Catch exceptions from game or system file execution
try:
    # Get the last game that was set to launch from `_run_on_boot`
    # file (ex: `Games/MyGame/main.py` if last selected in launcher)
    # (set in a file so that device can hard reset to clear memory)
    run_on_boot = get_run_on_boot()

    # If there's nothing to launch by the launcher, use 2nd argument
    # (if we have one) to launch a game (convenient on UNIX)
    if run_on_boot == None and len(sys.argv) >= 2:
        run_on_boot = sys.argv[1]

    # If nothing to launch from `_run_on_boot` and there weren't any
    # passed arguments, run the intro splash and then the launcher
    if run_on_boot == None:
        execfile("system/splash/show_splash.py")   # TODO: don't show this on launcher reset
        execfile("system/launcher/launcher.py")

    # Finally, if there is something to launch, launch it
    if run_on_boot != None:
        # Get the path to the directory containing the file
        dir = dirname(run_on_boot)

        # Add the path to the game file to PATH (so imports of game modules work)
        # and change to the game directory (so that opening files in the game dir works)
        sys.path.append(f"{ROOT_DIR}/{dir}")
        os.chdir(dir)

        # Launch the `_run_on_boot` file
        execfile(f"{ROOT_DIR}/{run_on_boot}")

        # Change back to the root directory after
        # the game ends and reset the device (hard)
        os.chdir(ROOT_DIR)
        thumby_reset(True)

except Exception as ex:
    # If an exception is raised from execution of a system or game file,
    # catch it, print, write to file, and then show the 'crash' screen
    sys.print_exception(ex)
    with open(f"{ROOT_DIR}/last_crash.txt", "w") as f:
        sys.print_exception(ex, f)
    execfile("system/crash.py")
