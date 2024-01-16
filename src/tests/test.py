# import machine
# p26_enable = machine.Pin(26, machine.Pin.OUT)
# p26_enable.value(0)

# p23_pwm = machine.Pin(23)
# pwm23 = machine.PWM(p23_pwm)
# pwm23.freq(500)
# pwm23.duty(512)

import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_physics import CircleCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import gc
import math
import os
import time
import random

print("dir(engine):", dir(engine))
print("dir(EmptyNode):", dir(EmptyNode))
print("dir(Sprite2DNode):", dir(Sprite2DNode))
print("dir(Rectangle2DNode):", dir(Rectangle2DNode))
print("dir(Vector3):", dir(Vector3))
print("dir(Rectangle):", dir(Rectangle))

# engine_debug.debug_enable_all()
engine.set_fps_limit(60)
# engine_debug.debug_enable_setting(engine_debug.debug_setting_performance)


background = TextureResource("128x128.bmp", True)
engine_draw.set_background(background)


class MyPhysicsNode(Physics2DNode):
    def __init__(self):
        super().__init__(self)

    def tick(self):
        if engine_input.is_a_pressed() or engine_input.is_b_pressed():
            self.velocity.x += (random.random()-0.5) * 3
            self.velocity.y += (random.random()-0.5) * 3

    def collision(self, collision_contact):
        # print("Collided!")
        print(engine.get_running_fps())
        # print(dir(collision_contact))
        # print(collision_contact.node)
        # print(collision_contact.node)
        # print("TEST")


circles = []
nodes = []

for i in range(4):
    p = Physics2DNode()
    c = Circle2DNode()
    p.add_child(c)

    p.collision_shape = CircleCollisionShape2D(16)
    p.bounciness = 2
    c.radius = 16
    p.position.x = 0
    p.position.y = 16 + (i * 32)
    p.dynamic = False

    circles.append(c)
    nodes.append(p)

for i in range(4):
    p = Physics2DNode()
    c = Circle2DNode()
    p.add_child(c)

    p.collision_shape = CircleCollisionShape2D(16)
    p.bounciness = 2
    c.radius = 16
    p.position.x = 128
    p.position.y = 16 + (i * 32)
    p.dynamic = False

    circles.append(c)
    nodes.append(p)

for i in range(4):
    p = Physics2DNode()
    c = Circle2DNode()
    p.add_child(c)

    p.collision_shape = CircleCollisionShape2D(16)
    p.bounciness = 2
    c.radius = 16
    p.position.x = 16 + (i * 32)
    p.position.y = 0
    p.dynamic = False

    circles.append(c)
    nodes.append(p)

for i in range(4):
    p = Physics2DNode()
    c = Circle2DNode()
    p.add_child(c)

    p.collision_shape = CircleCollisionShape2D(16)
    p.bounciness = 2
    c.radius = 16
    p.position.x = 16 + (i * 32)
    p.position.y = 128
    p.dynamic = False

    circles.append(c)
    nodes.append(p)



for i in range(16):
    circle = Circle2DNode()
    circle.radius = 6
    circle.color = random.randint(5000, 65500)
    ball = MyPhysicsNode()
    ball.collision_shape = CircleCollisionShape2D(6)
    ball.add_child(circle)
    ball.bounciness = 3
    ball.position.x = random.randint(64-40, 64+40)
    ball.position.y = random.randint(64-40, 64+40)

    circles.append(circle)
    nodes.append(ball)


# p0 = Physics2DNode()
# p1 = Physics2DNode()
# p2 = MyPhysicsNode()

# c0.radius = 5
# c1.radius = 5
# c2.radius = 10

# c0.color = 0b1111100000000000
# c1.color = 0b1111111111111111
# c2.color = 0b0000000000011111

# p0.add_child(c0)
# p1.add_child(c1)
# p2.add_child(c2)

# p0.collision_shape = CircleCollisionShape2D(5)
# p0.position.x = 64-32
# p0.position.y = 64
# p0.velocity.x = 1

# p1.collision_shape = CircleCollisionShape2D(5)
# p1.position.x = 64+32
# p1.position.y = 64
# p1.velocity.x = -1

# p2.collision_shape = CircleCollisionShape2D(10)
# p2.position.x = 64
# p2.position.y = 64+32
# p2.velocity.y = -1.5
# p2.mass = 1
# p2.dynamic = False

camera = CameraNode()
engine.start()


# # engine_debug.debug_enable_all()
# # engine_debug.debug_enable_setting(engine_debug.debug_setting_warnings)
# engine_debug.debug_enable_setting(engine_debug.debug_setting_performance)

# background = TextureResource("128x128.bmp", True)
# engine_draw.set_background(background)
# # engine_draw.set_background_color(engine_draw.skyblue)

# texture0 = TextureResource("32x32.bmp", True)
# texture1 = TextureResource("sheet.bmp")


# switched = False
# just_pressed = False
# rot_speed = 0.005

# class MySprite(Sprite2DNode):
#     def __init__(self):
#         super().__init__(self, texture1)
#         # self.transparent_color = engine_draw.black
#         self.scale.x = 1
#         self.scale.y = 1
    
#     def tick(self):
#         global rot_speed
#         global texture0
#         global texture1
#         global switched
#         global just_pressed
#         if engine_input.is_bumper_right_pressed():
#             rot_speed -= 0.0001
#         if engine_input.is_bumper_left_pressed():
#             rot_speed += 0.0001
        
#         if engine_input.is_a_pressed():
#             self.scale.x += 0.01
#             self.scale.y += 0.01
#         if engine_input.is_b_pressed():
#             self.scale.x -= 0.01
#             self.scale.y -= 0.01

#         if engine_input.is_dpad_up_pressed():
#             self.position.y -= 0.1
#         if engine_input.is_dpad_down_pressed():
#             self.position.y += 0.1
#         if engine_input.is_dpad_left_pressed():
#             self.position.x -= 0.1
#         if engine_input.is_dpad_right_pressed():
#             self.position.x += 0.1

#         if just_pressed == False and engine_input.is_menu_pressed():
#             just_pressed = True
#             if switched == False:
#                 switched = True
#                 self.texture_resource = texture1
#             else:
#                 switched = False
#                 self.texture_resource = texture0
        
#         if just_pressed == True and engine_input.is_menu_pressed() == False:
#             just_pressed = False
#         # self.rotation = (3*math.pi)/4
#         self.rotation = 0


# sprite = MySprite()
# sprite.position.x = 64
# sprite.position.y = 64
# sprite.fps = 0.5
# sprite.frame_count_x = 3
# sprite.frame_count_y = 3

# sprite2 = Sprite2DNode(texture0)
# sprite2.position.x = 32
# sprite2.scale.x = 0.5
# sprite2.scale.y = 0.5
# sprite2.transparent_color = engine_draw.black
# sprite.add_child(sprite2)

# sprite3 = Sprite2DNode(texture0)
# sprite3.position.x = -32
# sprite3.scale.x = 0.5
# sprite3.scale.y = 0.5
# sprite3.transparent_color = engine_draw.black
# sprite.add_child(sprite3)

# sprite4 = Sprite2DNode(texture0)
# sprite4.position.y = 32
# sprite4.scale.x = 0.5
# sprite4.scale.y = 0.5
# sprite4.transparent_color = engine_draw.black
# sprite.add_child(sprite4)

# sprite5 = Sprite2DNode(texture0)
# sprite5.position.y = -32
# sprite5.scale.x = 0.5
# sprite5.scale.y = 0.5
# sprite5.transparent_color = engine_draw.black
# sprite.add_child(sprite5)

# camera = CameraNode()

# engine.start()


# node = Rectangle2DNode()

# physics_node_6 = Physics2DNode()
# physics_node_6.physics_shape = engine_physics.PhysicsShapeCircle()
# physics_node_6.physics_shape.radius = 6
# physics_node_6.position.x = 128/2 + 32
# physics_node_6.position.y = 45
# physics_node_6.velocity.x = -0.5
# physics_node_6.compute_mass(1.0)
# circle_2 = Circle2DNode()
# circle_2.color = 0b11111000000000000
# circle_2.radius = 6
# circle_2.add_child(node)
# physics_node_6.add_child(circle_2)


# physics_ground = Physics2DNode()
# physics_ground.physics_shape = engine_physics.PhysicsShapeRectangle()
# physics_ground.physics_shape.width = 16
# physics_ground.physics_shape.height = 16
# physics_ground.position.x = 75
# physics_ground.position.y = 128-16
# physics_ground.compute_mass(1.0)
# # physics_ground.acceleration.y = -0.00981
# physics_ground.velocity.y = -0.85
# rectangle_ground = Rectangle2DNode()
# rectangle_ground.width = 16
# rectangle_ground.height = 16
# physics_ground.add_child(rectangle_ground)


# camera = CameraNode()

# # engine.start()

# engine.init()
# # time.sleep(0.5)

# for i in range(55):
#     engine.tick()
#     # print("1")
#     # time.sleep(0.3)

# while True:
#     engine.tick()
#     # time.sleep(0.3)
#     # print("2")

# print("HI2!")