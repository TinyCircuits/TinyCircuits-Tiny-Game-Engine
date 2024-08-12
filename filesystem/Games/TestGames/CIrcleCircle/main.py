import engine_main

import engine
from engine_math import Vector2
from engine_nodes import PhysicsCircle2DNode, CameraNode

c0 = PhysicsCircle2DNode(position=Vector2(-10, 0), outline=True)
c0 = PhysicsCircle2DNode(position=Vector2(10, 0), outline=True)

c = CameraNode()

engine.start()