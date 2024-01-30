import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
import engine_audio
from engine_physics import EmptyPolyCollisionShape2D, RectanglePolyCollisionShape2D, HexagonPolyCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Polygon2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource
import math

# engine_debug.debug_enable_all()
wave0 = WaveSoundResource("cow_s16.wav")
wave1 = WaveSoundResource("piano.wav")
# engine_debug.debug_enable_setting(engine_debug.debug_setting_performance)

engine_audio.play(wave0, 0, True)
engine_audio.play(wave1, 1, True)

# texture = TextureResource("32x32.bmp")

# class MySprite(Sprite2DNode):
#     def __init__(self):
#         super().__init__(self, texture)
    
#     # def tick(self):
#     #     if engine_input.is_dpad_up_pressed():
#     #         self.position.y -= 1
#     #     if engine_input.is_dpad_down_pressed():
#     #         self.position.y += 1
        
#     #     if engine_input.is_dpad_left_pressed():
#     #         self.position.x -= 1
#     #     if engine_input.is_dpad_right_pressed():
#     #         self.position.x += 1
        
        
#     #     if engine_input.is_bumper_left_pressed():
#     #         self.rotation += 0.1
#     #     if engine_input.is_bumper_right_pressed():
#     #         self.rotation -= 0.1




# circle = Circle2DNode()
# rectangle = Rectangle2DNode()
# sprite = MySprite()
# polygon = Polygon2DNode()
# polygon.vertices.append(Vector2(-10, 10))
# polygon.vertices.append(Vector2(10, 10))
# polygon.vertices.append(Vector2(10, -10))
# polygon.vertices.append(Vector2(-10, -10))

# circle.color = 0b1111100000000000
# rectangle.color = 0b1111101001001001
# sprite.transparent_color = engine_draw.black

# circle.position = Vector2(0, -32)
# rectangle.position = Vector2(32, 0)
# sprite.position = Vector2(0, 32)
# polygon.position = Vector2(-32, 0)


# class MyCam(CameraNode):
#     def __init__(self):
#         super().__init__(self)
    
#     def tick(self):
#         if engine_input.is_a_pressed():
#             self.zoom -= 0.1
#         if engine_input.is_b_pressed():
#             self.zoom += 0.1
        
#         if engine_input.is_dpad_up_pressed():
#             self.position.y -= 1
#         if engine_input.is_dpad_down_pressed():
#             self.position.y += 1
        
#         if engine_input.is_dpad_left_pressed():
#             self.position.x -= 1
#         if engine_input.is_dpad_right_pressed():
#             self.position.x += 1
        
        
#         if engine_input.is_bumper_left_pressed():
#             self.rotation.z += 0.1
#         if engine_input.is_bumper_right_pressed():
#             self.rotation.z -= 0.1


# camera = MyCam()
# camera.zoom = 0.5
# camera.position = Vector2(0, 0)
# # sprite.add_child(camera)

engine.start()