import engine

import os
from system.util import file_exists, read_and_delete, delete_file

ROOT_DIR = engine.root_dir()

LAUNCHER_STATE_FILE = f"{ROOT_DIR}/_launcher_state"
LAUNCHER_STATE_HELD_FILE = f"{LAUNCHER_STATE_FILE}__"


def has_launcher_state():
    """Returns whether the launcher state file exists."""
    return file_exists(LAUNCHER_STATE_FILE)


def get_launcher_state():
    """Returns the launcher state from the file and also deletes it. Returns None if it didn't exist."""
    delete_file(LAUNCHER_STATE_HELD_FILE)
    return read_and_delete(LAUNCHER_STATE_FILE)


def set_launcher_state(state):
    """Saves the launcher state to a file."""
    with open(LAUNCHER_STATE_FILE, "w") as f:
        f.write(state)


class hold_launcher_state:
    def __enter__(self):
        delete_file(LAUNCHER_STATE_HELD_FILE)
        self.held = file_exists(LAUNCHER_STATE_FILE)
        if self.held:
            os.rename(LAUNCHER_STATE_FILE, LAUNCHER_STATE_HELD_FILE)
        return self.held

    def __exit__(self, exc_type, exc_value, traceback):
        if self.held:
            # Restore the state if no exception, or an Exception. Skip restoring if e.g. SystemExit was raised.
            if exc_value is None or isinstance(exc_value, Exception):
                delete_file(LAUNCHER_STATE_FILE)
                if file_exists(LAUNCHER_STATE_HELD_FILE):
                    os.rename(LAUNCHER_STATE_HELD_FILE, LAUNCHER_STATE_FILE)
        # Don't suppress the exception.
        return False