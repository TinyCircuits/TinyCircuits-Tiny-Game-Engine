import engine_main

import engine
from engine_math import Vector2

from engine_nodes import PhysicsRectangle2DNode

class MyPhysics(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)

    def physics_tick(self, dt_s):
        print("This happens before collision callbacks or tick() in other nodes")

p = MyPhysics()

engine.start()