import machine
import time
from system.util import is_thumby

if is_thumby():
    # Wait for the screen to start. After a soft reset it should still be initialised.
    if machine.reset_cause() != machine.SOFT_RESET:
        time.sleep_ms(650)
