import engine
import engine_draw
from engine_math import Vector2
from engine_nodes import Circle2DNode, CameraNode
from engine_animation import Tween, ONE_SHOT, LOOP

print("Tween test!")

value = 0.0

t0 = Tween()
t1 = Tween()

circle0 = Circle2DNode(color=engine_draw.red, position=Vector2(-30, 30))
circle1 = Circle2DNode(color=engine_draw.green, position=Vector2(30, -30))

# t0.play(circle0.position.x, -15.0, 15.0, 500.0, LOOP)
# t1.play(circle0.position.y, -15.0, 15.0, 500.0, LOOP)

circle0.position.x = circle0.position.x + 50

camera = CameraNode()

# engine.start()
while True:
    # circle0.position.x = value
    # circle0.position.y = value
    engine.tick()