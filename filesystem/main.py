# main.py: This is the first file to run when the system boots up
import engine_main
import engine

import os
import sys
from system.run_on_boot import get_run_on_boot
from system.util import file_exists, dirname
from system.launcher_state import has_launcher_state, hold_launcher_state

ROOT_DIR = engine.root_dir()

# Catch exceptions from game or system file execution
try:
    # Always append path for Thumby Legacy files so
    # that the legacy modules can be imported
    sys.path.append(f"{ROOT_DIR}/lib")

    # Get the last game that was set to launch from `_run_on_boot`
    # file (ex: `Games/MyGame/main.py` if last selected in launcher)
    # (set in a file so that device can hard reset to clear memory)
    run_on_boot = get_run_on_boot()

    # If there's nothing to launch, use 2nd argument
    # (if we have one) to launch a game (convenient on UNIX)
    if run_on_boot is None and len(sys.argv) >= 2:
        run_on_boot = sys.argv[1]

    # If there's nothing to launch right away and there's
    # no launcher state file (indicating the launcher
    # has not launched before), show the splash
    if run_on_boot is None and not has_launcher_state():
        execfile("system/splash/show_splash.py")

    # If nothing to launch from `_run_on_boot` show launcher
    if run_on_boot is None:
        execfile("system/launcher/launcher.py")

    # Finally, if there is something to launch, launch it
    if run_on_boot is not None:
        # Get the path to the directory containing the file
        dir = dirname(run_on_boot)

        # Add the path to the game file to PATH (so imports of game modules work)
        # and change to the game directory (so that opening files in the game dir works)
        sys.path.append(f"{ROOT_DIR}/{dir}")
        os.chdir(dir)
        import engine_save
        engine_save._init_saves_dir(f"/Saves/{dir}")

        # Launch the `_run_on_boot` file while holding
        # the launcher state so that the state file is
        # deleted on exception
        try:
            with hold_launcher_state():
                execfile(f"{ROOT_DIR}/{run_on_boot}")
        finally:
            os.chdir(ROOT_DIR)

        # Change back to the root directory after
        # the game ends and reset the device (hard)
        os.chdir(ROOT_DIR)
        engine.reset()

except Exception as ex:
    # If an exception is raised from execution of a system or game file,
    # catch it, print, write to file, and then show the 'crash' screen
    sys.print_exception(ex)
    with open(f"{ROOT_DIR}/last_crash.txt", "w") as f:
        sys.print_exception(ex, f)
    execfile("system/crash.py")
