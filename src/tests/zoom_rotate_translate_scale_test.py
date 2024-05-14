import engine
import engine_draw
import engine_debug
import engine_io
import engine_physics
from engine_physics import EmptyPolyCollisionShape2D, RectanglePolyCollisionShape2D, HexagonPolyCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Polygon2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import math

engine.set_fps_limit(60)

texture = TextureResource("32x32.bmp")

circle = Circle2DNode()
rectangle = Rectangle2DNode()
sprite = Sprite2DNode(texture)
polygon = Polygon2DNode()
polygon.vertices.append(Vector2(-10, 10))
polygon.vertices.append(Vector2(10, 10))
polygon.vertices.append(Vector2(10, -10))
polygon.vertices.append(Vector2(-10, -10))

circle.color = 0b1111100000000000
rectangle.color = 0b1111101001001001

circle.position = Vector2(0, -32)
rectangle.position = Vector2(32, 0)
sprite.position = Vector2(0, 32)
polygon.position = Vector2(-32, 0)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        if engine_io.is_dpad_up_pressed():
            self.position.y -= 1
        if engine_io.is_dpad_down_pressed():
            self.position.y += 1
        
        if engine_io.is_dpad_left_pressed():
            self.position.x -= 1
        if engine_io.is_dpad_right_pressed():
            self.position.x += 1
        
        if engine_io.is_a_pressed():
            self.zoom -= 0.1
        if engine_io.is_b_pressed():
            self.zoom += 0.1
        
        if engine_io.is_bumper_left_pressed():
            self.rotation.z -= 0.25
        if engine_io.is_bumper_right_pressed():
            self.rotation.z += 0.25

        print(self.position.x, self.position.y, self.zoom)


camera = MyCam()
camera.zoom = 1
engine.start()