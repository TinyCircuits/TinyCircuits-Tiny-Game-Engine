import os
import random
from system.util import is_file, is_dir

def show_splash():
    """Run random /system/splash/custom_splash_scripts/*.py, if there are any, otherwise /system/splash/splash.py"""

    # If there is a custom_splash dir, run a random file from it (if there are any)
    if is_dir("system/splash/custom_splash_scripts"):
        files = [f for f in os.listdir("system/splash/custom_splash_scripts") if f.endswith(".py")]
        if files:
            execfile(f"system/splash/custom_splash_scripts/{random.choice(files)}")
            return
    
    execfile("system/splash/splash.py")

show_splash()
