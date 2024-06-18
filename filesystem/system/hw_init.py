import time
from system.util import is_thumby

if is_thumby():
    # Wait for the screen to start.
    # TODO: Consider checking if this is necessary also after a soft reset, and distinguishing between the two.
    time.sleep_ms(650)
