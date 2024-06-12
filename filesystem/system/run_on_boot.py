import os
import errno
from system.root_dir import ROOT_DIR

RUN_ON_BOOT_FILE = f"{ROOT_DIR}/_run_on_boot"


def get_run_on_boot():
    """Reads the run on boot file and also deletes it. Returns None if it didn't exist."""
    try:
        with open(RUN_ON_BOOT_FILE) as f:
            run_on_boot = f.read()
        os.remove(RUN_ON_BOOT_FILE)
        return run_on_boot
    except OSError as ex:
        if ex.errno == errno.ENOENT:
            return None
        raise


def set_run_on_boot(file_path):
    """Saves a file path to be run on boot, and resets the machine."""
    with open(RUN_ON_BOOT_FILE, "w") as f:
        f.write(file_path)
