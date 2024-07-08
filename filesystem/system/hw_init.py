import time
from system.util import IS_THUMBY

if IS_THUMBY:
    # Wait for the screen to start.
    # TODO: Consider checking if this is necessary also after a soft reset, and distinguishing between the two.
    time.sleep_ms(650)
