import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
import engine_audio
from engine_physics import EmptyPolyCollisionShape2D, RectanglePolyCollisionShape2D, HexagonPolyCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Polygon2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode, Text2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource, FontResource
import math
import gc

# engine_debug.enable_all()
# engine.set_fps_limit(30)
# engine_debug.enable_setting(engine_debug.performance)
font9 = FontResource("9pt-roboto-font.bmp")
# font12 = FontResource("12pt-roboto-font.bmp")

# class MyText(Text2DNode):
#     def __init__(self):
#         super().__init__(self, Vector2(0, 0), font9)
#         self.text = "Hello World!\nLine 2\nLine 3\nLine 4                       hi"



text = Text2DNode(text="Hello World!\nLine 2\nLine 3", font=font9, scale=Vector2(1.0, 1.0), position=Vector2(0, -0))
# text = MyText()
# text.rotation = math.pi/4
# print(text.width)
# text.width = 1


texture = TextureResource("32x32.bmp")


class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        self.rotation += 0.001


circle = MyCircle()
circle.outline = True
circle.radius = 25
rectangle = Rectangle2DNode()
rectangle.width = 30
rectangle.height = 15
rectangle.outline = True
sprite = Sprite2DNode(texture=texture, position=Vector2(0, 0), scale=Vector2(1.0, 1.0))
polygon = Polygon2DNode()
polygon.vertices.append(Vector2(-15, 10))
polygon.vertices.append(Vector2(10, 10))
polygon.vertices.append(Vector2(-5, 15))
polygon.vertices.append(Vector2(10, -10))
polygon.vertices.append(Vector2(-10, -15))
polygon.outline = True

circle.color = 0b1111100000000000
rectangle.color = 0b1111101001001001
sprite.transparent_color = engine_draw.black

circle.position = Vector2(0, 0)
rectangle.position = Vector2(32, 0)
sprite.position = Vector2(0, 32)
polygon.position = Vector2(-32, 0)
text.position = Vector2(0, -36)

circle.add_child(rectangle)
circle.add_child(sprite)
circle.add_child(polygon)
circle.add_child(text)

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        if engine_input.check_pressed(engine_input.A):
            self.zoom -= 0.01
        if engine_input.check_pressed(engine_input.B):
            self.zoom += 0.01
        
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
# camera.add_child(polygon)

engine.start()