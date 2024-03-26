import engine
import engine_draw
from engine_math import Vector2
from engine_nodes import Circle2DNode, CameraNode
from engine_animation import Tween, ONE_SHOT, LOOP
# import engine_debug

# engine_debug.enable_all()

print("Tween test!")

value = 0.0

t0 = Tween()
t1 = Tween()

circle0 = Circle2DNode(color=engine_draw.red, position=Vector2(-30, 30))
circle1 = Circle2DNode(color=engine_draw.green, position=Vector2(30, -30))

t0.start(circle0.position, Vector2(-64, -64), Vector2( 64, 64), 1500.0, LOOP)
t1.start(circle1.position, Vector2( 64, -64), Vector2(-64, 64), 1500.0, LOOP)

# circle0.position.x = circle0.position.y + 1



camera = CameraNode()

# engine.start()
while True:
    # print(circle0.position.x, circle0.position.y)
    # circle0.position.x = value
    # circle0.position.y = value
    engine.tick()