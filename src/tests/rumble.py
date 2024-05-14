import engine_main

import engine
import engine_io

engine_io.rumble(1.0)

while True:
    print("TEST")
    engine.tick()