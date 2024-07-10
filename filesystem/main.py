import engine_main

import os
import sys
from system.launcher_state import has_launcher_state, hold_launcher_state
from system.root_dir import ROOT_DIR
from system.run_on_boot import get_run_on_boot
from system.util import file_exists, dirname, thumby_reset

try:

    run_on_boot = get_run_on_boot()
    # Allow running an app by command line.
    if not run_on_boot and len(sys.argv) >= 2:
        run_on_boot = sys.argv[1]
    if run_on_boot and file_exists(run_on_boot):
        execfile("system/game_menu.py")
        dir = dirname(run_on_boot)
        sys.path.append(f"{ROOT_DIR}/{dir}")
        os.chdir(dir)
        try:
            with hold_launcher_state():
                execfile(f"{ROOT_DIR}/{run_on_boot}")
        finally:
            os.chdir(ROOT_DIR)
    else:
        if not has_launcher_state():
            execfile("system/splash.py")
        execfile("system/launcher.py")
    # Perform hard reset after the app finishes.
    thumby_reset(True)

except Exception as ex:
    sys.print_exception(ex)
    # Catch any exception, including one originating from a system file.
    with open("last_crash.txt", "w") as f:
        sys.print_exception(ex, f)
    execfile("system/crash.py")
