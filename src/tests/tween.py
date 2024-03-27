import engine
import engine_draw
from engine_draw import Color
from engine_math import Vector2, Vector3
from engine_nodes import Circle2DNode, CameraNode, VoxelSpaceNode
from engine_animation import Tween, ONE_SHOT, LOOP, PING_PONG
from engine_resources import TextureResource
import gc
# import engine_debug

# engine_debug.enable_all()

print(engine_draw.black)

a = engine_draw.black
a.r = 0.5
print(a)

engine_draw.set_background_color(engine_draw.skyblue)
C18W = TextureResource("C18W.bmp", True)
D18 = TextureResource("D18.bmp", True)


vox = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox.position.x = -75
vox.position.y = -75
vox.position.z = -75
vox.height_scale = 40


# print(vox.position)
print("Tween test!")

a = vox.position.x
vox.position.x = vox.position.x + 1

print(a, vox.position)

print("Tween test!")

t0 = Tween()
t1 = Tween()
t2 = Tween()
t3 = Tween()

circle0 = Circle2DNode(color=engine_draw.red, position=Vector2(-30, 30))
circle1 = Circle2DNode(color=engine_draw.green, position=Vector2(30, -30))


t0.start(circle0, "position", Vector2( -64, -64), Vector2(64, 64), 3000.0, PING_PONG)
t1.start(circle1, "position", Vector2( 64, -64), Vector2(-64, 64), 3000.0, PING_PONG)


camera = CameraNode()
camera.add_child(circle0)
camera.add_child(circle1)

t2.start(camera, "position", Vector3(50, 32, 50), Vector3(-50, 200, -50), 3000.0, PING_PONG)

t3.start(circle0, "color", Color(1, 0, 0), Color(0, 1, 0), 3000.0, PING_PONG)

# engine.start()
while True:
    # print(circle0.position.x, circle0.position.y)
    # circle0.position.x = value
    # circle0.position.y = value
    engine.tick()
    gc.collect()