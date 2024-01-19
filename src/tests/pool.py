import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_physics import CircleCollisionShape2D, RectangleCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource


engine.set_fps_limit(60)
engine_physics.set_physics_fps_limit(60)


top_wall_physics = Physics2DNode()
bottom_wall_physics = Physics2DNode()
left_wall_physics = Physics2DNode()
right_wall_physics = Physics2DNode()

top_wall_rect = Rectangle2DNode()
bottom_wall_rect = Rectangle2DNode()
left_wall_rect = Rectangle2DNode()
right_wall_rect = Rectangle2DNode()

top_wall_rect.width = 64
top_wall_rect.height = 5
bottom_wall_rect.width = 64
bottom_wall_rect.height = 5
left_wall_rect.width = 5
left_wall_rect.height = 128
right_wall_rect.width = 5
right_wall_rect.height = 128

top_wall_rect.color = 0b1111100000000000
bottom_wall_rect.color = 0b1111100000000000
left_wall_rect.color = 0b1111100000000000
right_wall_rect.color = 0b1111100000000000

top_wall_physics.collision_shape = RectangleCollisionShape2D(64, 5)
bottom_wall_physics.collision_shape = RectangleCollisionShape2D(64, 5)
left_wall_physics.collision_shape = RectangleCollisionShape2D(5, 128)
right_wall_physics.collision_shape = RectangleCollisionShape2D(5, 128)

top_wall_physics.add_child(top_wall_rect)
bottom_wall_physics.add_child(bottom_wall_rect)
left_wall_physics.add_child(left_wall_rect)
right_wall_physics.add_child(right_wall_rect)

top_wall_physics.dynamic = False
bottom_wall_physics.dynamic = False
left_wall_physics.dynamic = False
right_wall_physics.dynamic = False

top_wall_physics.position = Vector2(128/2, 0)
bottom_wall_physics.position = Vector2(128/2, 128)
left_wall_physics.position = Vector2(32, 128/2)
right_wall_physics.position = Vector2(128-32, 128/2)


top_wall_physics.bounciness = 2
bottom_wall_physics.bounciness = 2
left_wall_physics.bounciness = 2
right_wall_physics.bounciness = 2

# texture = TextureResource("32x32.bmp", True)

# class MyPhysicsTestNode(Physics2DNode):
#     def __init__(self):
#         super().__init__(self)
    
#     def tick(self):
#         if engine_input.is_dpad_up_pressed():
#             self.velocity.y -= 0.1
#         if engine_input.is_dpad_down_pressed():
#             self.velocity.y += 0.1
#         if engine_input.is_dpad_left_pressed():
#             self.velocity.x -= 0.1
#         if engine_input.is_dpad_right_pressed():
#             self.velocity.x += 0.1
        
#         if engine_input.is_a_pressed():
#             self.collision_shape.radius += 0.01*2
#         if engine_input.is_b_pressed():
#             self.collision_shape.radius -= 0.01*2


# class MySpriteTestNode(Sprite2DNode):
#     def __init__(self, texture):
#         super().__init__(self, texture)
#         self.rot_speed = 0.0

#         self.sprites = []
#         self.nodes = []

#     def tick(self):
#         if engine_input.is_bumper_right_pressed():
#             self.rot_speed  -= 0.001
#         if engine_input.is_bumper_left_pressed():
#             self.rot_speed  += 0.001
        
#         if engine_input.is_a_pressed():
#             self.scale.x += 0.01
#             self.scale.y += 0.01
#         if engine_input.is_b_pressed():
#             self.scale.x -= 0.01
#             self.scale.y -= 0.01
        
#         self.rotation += self.rot_speed


# sprite = MySpriteTestNode(texture)
# sprite.transparent_color = engine_draw.black
# # sprite.frame_count_x = 4
# # sprite.fps = 5
# sprite_physics = MyPhysicsTestNode()
# sprite_physics.position = Vector2(64, 64)
# sprite_physics.collision_shape = CircleCollisionShape2D(16)
# sprite_physics.add_child(sprite)
# sprite_physics.bounciness = 2


camera = CameraNode()
engine.start()