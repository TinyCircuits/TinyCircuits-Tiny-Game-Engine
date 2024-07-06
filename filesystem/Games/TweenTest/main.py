import engine_main
import engine
import math

from engine_animation import Tween, ONE_SHOT, EASE_ELAST_IN_OUT
from engine_nodes import Rectangle2DNode, CameraNode
from engine_math import Vector2, Vector3

cam = CameraNode()
engine.fps_limit(60)

class MyRect(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.tween = Tween()

r0 = MyRect()
r0.position.x = -10
r0.position.y = 0
r0.rotation = 0.0

r1 = MyRect()
r1.position.x = 10
r1.position.y = 0
r1.rotation = 0.0


r1.tween.start(r1, "rotation", 0.0, 2*math.pi, 1000, 1.0, ONE_SHOT, EASE_ELAST_IN_OUT)
# r1.tween.start(r1, "position", r1.position, Vector2(30, 30), 1000, 1.0, ONE_SHOT, EASE_ELAST_IN_OUT)


engine.start()