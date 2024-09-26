# Test #1
import engine_main
import engine
from engine_nodes import Circle2DNode, CameraNode
engine.disable_fps_limit()

circle = Circle2DNode()
camera = CameraNode()

ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[circle_node_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")


# Test #2
import engine_main
import engine
from engine_nodes import Line2DNode, CameraNode
engine.disable_fps_limit()

line = Line2DNode()
camera = CameraNode()
ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[line_node_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")


# Test #3
import engine_main
import engine
import engine
import engine_draw
import engine_debug
import engine_io
import engine_physics
import engine_audio
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Text2DNode, Line2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource, FontResource
import math
import gc

engine.disable_fps_limit()


line = Line2DNode(color=engine_draw.orange, start=Vector2(40, -15), end=Vector2(40, 15), thickness=1)
line.opacity = 0.5

font9 = FontResource("9pt-roboto-font.bmp")


class MyText(Text2DNode):
    def __init__(self):
        super().__init__(self, Vector2(0, 0), font9)
        self.text = "Hello World!\nLine 2\nLine 3\nLine 4                       hi"

text = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(2.0, 2.0), position=Vector2(0, -0))
text.opacity = 0.75

text0 = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(1.0, 1.0), position=Vector2(0, -0))


texture = TextureResource("32x32.bmp")


class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        self.rotation += 0.005


circle = MyCircle()
circle.radius = 25
circle.opacity = 0.35
rectangle = Rectangle2DNode()
rectangle.width = 30
rectangle.height = 15
rectangle.opacity = 0.85
rectangle.outline = True
sprite = Sprite2DNode(texture=texture, position=Vector2(0, 0), scale=Vector2(1.0, 1.0))

circle.color = engine_draw.red
rectangle.color = engine_draw.yellow
sprite.transparent_color = engine_draw.black
sprite.opacity = 0.4

circle.position = Vector2(10, 0)
rectangle.position = Vector2(32, 0)
sprite.position = Vector2(0, 32)
text.position = Vector2(0, -36)
text0.position = Vector2(0, -36)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        if engine_io.A.is_pressed:
            self.zoom -= 0.005
        if engine_io.B.is_pressed:
            self.zoom += 0.005
        
        if engine_io.UP.is_pressed:
            self.position.y -= 0.25
        if engine_io.DOWN.is_pressed:
            self.position.y += 0.25
        
        if engine_io.LEFT.is_pressed:
            self.position.x -= 0.25
        if engine_io.RIGHT.is_pressed:
            self.position.x += 0.25
        
        if engine_io.LB.is_pressed:
            self.rotation.z += 0.005
        if engine_io.RB.is_pressed:
            self.rotation.z -= 0.005



camera = MyCam()
camera.zoom = 1


circle.add_child(rectangle)
circle.add_child(sprite)
circle.add_child(text)
circle.add_child(text0)
circle.add_child(line)

cursor = Circle2DNode(radius=7, color=engine_draw.green, outline=False, position=Vector2(0, 0), opacity=0.2)

camera.add_child(cursor)

ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[test_opacity_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")


# Test #4
import engine_main
import engine
import engine_draw
import engine_debug
import engine_io
import engine_physics
import engine_audio
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Text2DNode, Line2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource, FontResource
import math
import gc

engine.disable_fps_limit()

line = Line2DNode(color=engine_draw.orange, start=Vector2(40, -15), end=Vector2(40, 15), thickness=1)
line.opacity = 1.0

font9 = FontResource("9pt-roboto-font.bmp")


class MyText(Text2DNode):
    def __init__(self):
        super().__init__(self, Vector2(0, 0), font9)
        self.text = "Hello World!\nLine 2\nLine 3\nLine 4                       hi"

text = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(2.0, 2.0), position=Vector2(0, -0))
text.opacity = 1.0

text0 = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(1.0, 1.0), position=Vector2(0, -0))


texture = TextureResource("32x32.bmp")


class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        self.rotation += 0.005


circle = MyCircle()
circle.radius = 25
circle.opacity = 1.0
rectangle = Rectangle2DNode()
rectangle.width = 30
rectangle.height = 15
rectangle.opacity = 1.0
rectangle.outline = True
sprite = Sprite2DNode(texture=texture, position=Vector2(0, 0), scale=Vector2(1.0, 1.0))

circle.color = engine_draw.red
rectangle.color = engine_draw.yellow
sprite.transparent_color = engine_draw.black
sprite.opacity = 1.0

circle.position = Vector2(10, 0)
rectangle.position = Vector2(32, 0)
sprite.position = Vector2(0, 32)
text.position = Vector2(0, -36)
text0.position = Vector2(0, -36)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        if engine_io.A.is_pressed:
            self.zoom -= 0.005
        if engine_io.B.is_pressed:
            self.zoom += 0.005
        
        if engine_io.UP.is_pressed:
            self.position.y -= 0.25
        if engine_io.DOWN.is_pressed:
            self.position.y += 0.25
        
        if engine_io.LEFT.is_pressed:
            self.position.x -= 0.25
        if engine_io.RIGHT.is_pressed:
            self.position.x += 0.25
        
        if engine_io.LB.is_pressed:
            self.rotation.z += 0.005
        if engine_io.RB.is_pressed:
            self.rotation.z -= 0.005



camera = MyCam()
camera.zoom = 1


circle.add_child(rectangle)
circle.add_child(sprite)
circle.add_child(text)
circle.add_child(text0)
circle.add_child(line)

cursor = Circle2DNode(radius=7, color=engine_draw.green, outline=False, position=Vector2(0, 0), opacity=1.0)

camera.add_child(cursor)


ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[test_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")


# Test #5
import engine_main

import engine
import engine_draw
from engine_nodes import VoxelSpaceNode, CameraNode
from engine_resources import TextureResource
import math

engine.disable_fps_limit()

engine_draw.set_background_color(engine_draw.skyblue)

C18W = TextureResource("C18W.bmp", True)
D18 = TextureResource("D18.bmp", True)

vox = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox.position.x = 200
vox.position.y = 0
vox.scale.y = 10


camera = CameraNode()
camera.position.x = 175
camera.position.y = 10
camera.position.z = 75
camera.view_distance = 350
camera.fov = 70 * (math.pi/180)

ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[vox_node_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")

engine.reset(True)