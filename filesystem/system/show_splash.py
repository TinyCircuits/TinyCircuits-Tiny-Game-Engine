import os
import random
from system.util import is_file, is_dir

def show_splash():
    """Run /splash.py or random /splash/*.py file, or fall back to /system/splash.py."""
    if is_file("splash.py"):
        execfile("splash.py")
        return
    if is_dir("splash"):
        files = [f for f in os.listdir("splash") if f.endswith(".py")]
        if files:
            execfile(f"splash/{random.choice(files)}")
            return
    execfile("system/splash.py")

show_splash()
