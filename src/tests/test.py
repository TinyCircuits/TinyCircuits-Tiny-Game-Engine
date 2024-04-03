import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
import engine_audio
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Text2DNode, Line2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource, FontResource
import math
import gc


engine.set_fps_limit(60)

# class MyLine(Line2DNode):
#     def __init__(self):
#         super().__init__(self)
#         self.color = engine_draw.purple


# my_line = MyLine()
# my_line.start.x = 20
# my_line.start.y = 50

# my_line.position.x = -64

line = Line2DNode(color=engine_draw.orange, start=Vector2(40, -15), end=Vector2(40, 15), thickness=1)
line.opacity = 0.6

# By default, nodes are at center of screen at 0,0
# engine_debug.enable_all()
# engine.set_fps_limit(60)
# engine_debug.enable_setting(engine_debug.performance)
font9 = FontResource("9pt-roboto-font.bmp")
# font12 = FontResource("12pt-roboto-font.bmp")

class MyText(Text2DNode):
    def __init__(self):
        super().__init__(self, Vector2(0, 0), font9)
        self.text = "Hello World!\nLine 2\nLine 3\nLine 4                       hi"

text = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(2.0, 2.0), position=Vector2(0, -0))
text.opacity = 0.5

text0 = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(1.0, 1.0), position=Vector2(0, -0))


texture = TextureResource("32x32.bmp")


class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        self.rotation += 0.005


circle = MyCircle()
# circle.outline = True
circle.radius = 25
circle.opacity = 0.25
rectangle = Rectangle2DNode()
rectangle.width = 30
rectangle.height = 15
rectangle.opacity = 0.5
rectangle.outline = True
sprite = Sprite2DNode(texture=texture, position=Vector2(0, 0), scale=Vector2(1.0, 1.0))

circle.color = engine_draw.red
rectangle.color = engine_draw.yellow
sprite.transparent_color = engine_draw.black
sprite.opacity = 0.3

circle.position = Vector2(10, 0)
rectangle.position = Vector2(32, 0)
sprite.position = Vector2(0, 32)
text.position = Vector2(0, -36)
text0.position = Vector2(0, -36)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        print(engine.get_running_fps())
        if engine_input.check_pressed(engine_input.A):
            self.zoom -= 0.005
        if engine_input.check_pressed(engine_input.B):
            self.zoom += 0.005
        
        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.position.y -= 0.25
        if engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.position.y += 0.25
        
        if engine_input.check_pressed(engine_input.DPAD_LEFT):
            self.position.x -= 0.25
        if engine_input.check_pressed(engine_input.DPAD_RIGHT):
            self.position.x += 0.25
        
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation.z += 0.005
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation.z -= 0.005



camera = MyCam()
camera.zoom = 1

# class FPSText(Text2DNode):
#     def __init__(self):
#         super().__init__(self)
#         self.font = font9
#         self.text = "0 FPS"
    
#     def tick(self):
#         self.text = str(int(engine.get_running_fps())) + " FPS"
#         self.position.x = -camera.viewport.width / 2 + self.width / 2 + 4
#         self.position.y = -camera.viewport.height / 2 + self.height / 2 + 1


# fps = FPSText()

circle.add_child(rectangle)
circle.add_child(sprite)
circle.add_child(text)
circle.add_child(text0)
circle.add_child(line)

cursor = Circle2DNode(radius=7, color=engine_draw.green, outline=False, position=Vector2(0, 0), opacity=0.5)

# camera.add_child(circle)
# camera.add_child(fps)
camera.add_child(cursor)
# camera.add_child(sprite)
# camera.add_child(rectangle)

engine.start()