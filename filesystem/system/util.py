import os
import errno
import sys

import hashlib

def directory_hash(dir, extra = []):
    sha = hashlib.sha256()

    for file in sorted(extra):
        print("Hashing "+file)
        file = open(file, 'rb')
        while chunk := file.read(8192):
            sha.update(chunk)
            
    #print(str(dirtree))
    '''
    for root, dir, manifest in dirtree:
        for file in sorted(manifest):
            if(file.endswith('.py')):
                print("Hashing "+file+" in "+str(dir))
                file = open(os.path.join(root, file), 'rb')
                while chunk := file.read(8192):
                    sha.update(chunk)
    '''
    def _traverse(path):
        # listdir returns names; we sort them for consistency with the build script
        items = sorted(os.listdir(path))
        
        for item in items:
            full_path = path + "/" + item if path != "/" else "/" + item
            stat = os.stat(full_path)
            
            if stat[0] & 0x4000:  # Check if it's a directory
                _traverse(full_path)
            elif item.endswith('.py'):
                with open(full_path, 'rb') as f:
                    print("Hashing "+str(full_path))
                    while True:
                        chunk = f.read(128)
                        if not chunk:
                            break
                        sha.update(chunk)
    _traverse(dir)
    
    digest = sha.digest()
    ret = 0
    for i in range(6):
        ret <<= 8
        ret |= digest[i]
    print("System scripts digest: "+str(ret))
    return ret


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