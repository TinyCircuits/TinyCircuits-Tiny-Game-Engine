import engine_main
import engine
from engine_animation import Delay

# Define custom Delay callback functions
def customTick(self, dt):
    print("Time since last tick (ms)", dt)

def customAfter(self):
    print("Delay over!", self.finished)

# Override callbacks through inheritance
class CustomDelay(Delay):
    def __init__(self):
        pass
    
    def tick(self, dt):
        customTick(self, dt)

    def after(self):
        customAfter(self)

# Create two delay objects, our custom class object and a native Delay object
customDelay = CustomDelay()
delay = Delay()

# Set callbacks like this, but .start(...) will override these if provided
delay.tick = customTick()
delay.after = customAfter()

# Override the manually set callback with the same function
delay.start(2000, customAfter)

# Don't override the custom callbacks overridden in the class and use the default delay
customDelay.start()

while True:
    # This needs to be called for Delay to work
    if engine.tick() is False:
        continue