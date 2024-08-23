import os
import errno
import machine
import sys

from system.root_dir import ROOT_DIR


def file_exists(path):
    try:
        os.stat(path)
        return True
    except OSError as ex:
        if ex.errno == errno.ENOENT:
            return False
        raise


def is_file(path):
    try:
        return os.stat(path)[0] & 0x4000 == 0
    except OSError as ex:
        if ex.errno == errno.ENOENT:
            return False
        raise


def is_dir(path):
    try:
        return os.stat(path)[0] & 0x4000 == 0x4000
    except OSError as ex:
        if ex.errno == errno.ENOENT:
            return False
        raise


def read_and_delete(path, default=None):
    """Reads the specified file and deletes it. Returns the default value if it didn't exist."""
    try:
        with open(path) as f:
            data = f.read()
        os.remove(path)
        return data
    except OSError as ex:
        if ex.errno == errno.ENOENT:
            return default
        raise


def delete_file(path):
    """Deletes the file, if it exists."""
    try:
        os.remove(path)
    except OSError as ex:
        if ex.errno != errno.ENOENT:
            raise


# Given a file path, return just the name of the file
def basename(path):
    return path[path.rfind("/") + 1 :]


# Given a file path, return the path to the directory the file is in
def dirname(path):
    index = path.rfind("/")
    return "/" if index < 0 else path[:index]