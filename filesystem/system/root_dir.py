import os

# The root dir. On the device it is "/", on emulator it might be a host system path.
#
# Usage:
#   path = f"{ROOT_DIR}/dir/file"
#
# On Thumby the path will now start with double slash, but this is fine.
#
# Note that to open a resource located at the root level, e.g. /assets/some_asset, you can
# simply specify the absolute path "/assets/some_asset" as an argument to a resource constructor.
# The ROOT_DIR is only needed when manipulating path for other purposes, e.g. `open()`.
ROOT_DIR = os.getcwd()
