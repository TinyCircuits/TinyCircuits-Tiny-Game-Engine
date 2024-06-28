import os

# The root dir. On the device it is "/", on emulator it might be a host system path.
#
# Usage:
#   path = f"{ROOT_DIR}/dir/file"
# On Thumby the path will now start with double slash, but this is fine.
ROOT_DIR = os.getcwd()
