import engine
import engine_draw
import engine_input
from engine_draw import Color
from engine_math import Vector2, Vector3
from engine_nodes import Circle2DNode, CameraNode, VoxelSpaceNode
from engine_animation import Tween, ONE_SHOT, LOOP, PING_PONG, EASE_CIRC_IN_OUT, EASE_BOUNCE_IN, EASE_BOUNCE_OUT
from engine_resources import TextureResource
import gc
import math
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



class AnimatedCircle():
    def __init__(self):
        self.circle = Circle2DNode(color=engine_draw.red, position=Vector2(-30, 30))
        self.tween = Tween()
        
        self.color_tween = Tween()
        self.color_tween.start(self.circle, "color", Color(1, 0, 0), Color(0, 1, 0))
    
    def tween_after(self, tween):
        self.tween.start(self.circle, "position", self.circle.position, Vector2(-64, -64), 3000.0, 1.0, ONE_SHOT)
        self.tween.after = None
    
    def start(self):
        self.tween.after = self.tween_after
        self.tween.start(self.circle, "position", Vector2( 64, -64), Vector2(-64, 64), 3000.0, 1.0, ONE_SHOT)



t1 = Tween()
t2 = Tween()
t3 = Tween()

circle1 = Circle2DNode(color=engine_draw.green, position=Vector2(30, -30))


t1.start(circle1, "position", Vector2( 64, -64), Vector2(-64, 64), 3000.0, 1.0, PING_PONG, EASE_BOUNCE_OUT)


ac = AnimatedCircle()
ac.start()


value = 0.0
t3.start(value, "", 0.0, 1.0, 3000.0, 1.0, LOOP, EASE_BOUNCE_IN)


class cam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.target = Vector3(75, 0, -36)
        self.intensity = 0.0

    def tick(self):
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.intensity += 0.05
            engine_input.rumble(self.intensity)
            print(self.intensity)
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.intensity -= 0.05
            engine_input.rumble(self.intensity)
            print(self.intensity)

        lookat_x = self.target.x - self.position.x
        lookat_y = self.target.y - self.position.y
        lookat_z = self.target.z - self.position.z

        len = math.sqrt((lookat_x*lookat_x) + (lookat_y*lookat_y) + (lookat_z*lookat_z))

        lookat_x = lookat_x / len
        lookat_y = lookat_y / len
        lookat_z = lookat_z / len

        self.rotation.x = math.asin(lookat_y)
        self.rotation.y = math.atan2(lookat_x, lookat_z)


camera = cam()
camera.add_child(ac.circle)
camera.add_child(circle1)

t2.start(camera, "position", Vector3(50, 32, 50), Vector3(-50, 200, -50), 3000.0, 0.5, PING_PONG, EASE_BOUNCE_IN)


# engine.start()
while True:
    # print(circle0.position.x, circle0.position.y)
    # circle0.position.x = value
    # circle0.position.y = value
    engine.tick()