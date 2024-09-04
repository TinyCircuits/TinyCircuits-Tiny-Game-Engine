import engine
from system.util import read_and_delete

ROOT_DIR = engine.root_dir()

RUN_ON_BOOT_FILE = f"{ROOT_DIR}/_run_on_boot"


def get_run_on_boot():
    """Reads the run on boot file and also deletes it. Returns None if it didn't exist."""
    return read_and_delete(RUN_ON_BOOT_FILE)


def set_run_on_boot(file_path):
    """Saves a file path to be run on boot, and resets the machine."""
    with open(RUN_ON_BOOT_FILE, "w") as f:
        f.write(file_path)
