import engine_main

import engine
import engine_draw
import engine_debug
import engine_io
import engine_physics
import engine_audio
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Text2DNode, Line2DNode, GUIButton2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource, FontResource
import math
import gc

engine.set_fps_limit(60)


line = Line2DNode(color=engine_draw.orange, start=Vector2(40, -15), end=Vector2(40, 15), thickness=1)
line.opacity = 1.0

font9 = FontResource("9pt-roboto-font.bmp")


class MyText(Text2DNode):
    def __init__(self):
        super().__init__(self, Vector2(0, 0), font9)
        self.text = "Hello World!\nLine 2\nLine 3\nLine 4                       hi"

text = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(2.0, 2.0), position=Vector2(0, -0), line_spacing = 2.0, letter_spacing=2)
text.opacity = 1.0

text0 = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(1.0, 1.0), position=Vector2(0, -0))


texture = TextureResource("32x32.bmp")


button0 = GUIButton2DNode(position=Vector2(-45,   0), font=font9, text="Button 0", rotation=0, scale=Vector2(0.75, 0.75), padding=2, outline=2, opacity=1.0)


class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        self.rotation += 0.005


circle = MyCircle()
circle.radius = 25
circle.opacity = 0.5
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
        print(engine.get_running_fps())
        if engine_io.check_pressed(engine_io.A):
            self.zoom -= 0.005
        if engine_io.check_pressed(engine_io.B):
            self.zoom += 0.005
        
        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.position.y -= 0.25
        if engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.position.y += 0.25
        
        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.position.x -= 0.25
        if engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.position.x += 0.25
        
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            self.rotation.z += 0.005
        if engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            self.rotation.z -= 0.005



camera = MyCam()
camera.zoom = 1


circle.add_child(rectangle)
circle.add_child(sprite)
circle.add_child(text)
circle.add_child(text0)
circle.add_child(line)
circle.add_child(button0)

cursor = Circle2DNode(radius=4, color=engine_draw.green, outline=True, position=Vector2(0, 0), opacity=1.0)

camera.add_child(cursor)

engine.start()