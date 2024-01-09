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
from engine_physics import PhysicsShapeRectangle
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import gc
import math
import os
import time

print("dir(engine):", dir(engine))
print("dir(EmptyNode):", dir(EmptyNode))
print("dir(Sprite2DNode):", dir(Sprite2DNode))
print("dir(Rectangle2DNode):", dir(Rectangle2DNode))
print("dir(Vector3):", dir(Vector3))
print("dir(Rectangle):", dir(Rectangle))

# engine_debug.debug_enable_all()
# engine_debug.debug_enable_setting(engine_debug.debug_setting_warnings)
# engine_debug.debug_enable_setting(engine_debug.debug_setting_performance)

# # print("HI!")

texture0 = TextureResource("32x32.bmp")
texture1 = TextureResource("rpi.bmp")
switched = False
just_pressed = False
rot_speed = 0.005

class MySprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self, texture0)
    
    def tick(self):
        global rot_speed
        global texture0
        global texture1
        global switched
        global just_pressed
        if engine_input.is_bumper_right_pressed():
            rot_speed += 0.0001
        if engine_input.is_bumper_left_pressed():
            rot_speed -= 0.0001
        
        if engine_input.is_a_pressed():
            self.scale.x += 0.01
            self.scale.y += 0.01
        if engine_input.is_b_pressed():
            self.scale.x -= 0.01
            self.scale.y -= 0.01

        if engine_input.is_dpad_up_pressed():
            self.position.y -= 0.1
        if engine_input.is_dpad_down_pressed():
            self.position.y += 0.1
        if engine_input.is_dpad_left_pressed():
            self.position.x -= 0.1
        if engine_input.is_dpad_right_pressed():
            self.position.x += 0.1

        if just_pressed == False and engine_input.is_menu_pressed():
            just_pressed = True
            if switched == False:
                switched = True
                self.texture_resource = texture1
            else:
                switched = False
                self.texture_resource = texture0
        
        if just_pressed == True and engine_input.is_menu_pressed() == False:
            just_pressed = False
        self.rotation += rot_speed


sprite = MySprite()
sprite.position.x = 64
sprite.position.y = 64

# sprite2 = MySprite()
# sprite2.position.x = 32
# sprite2.scale.x = 0.5
# sprite2.scale.y = 0.5
# sprite.add_child(sprite2)

# sprite3 = MySprite()
# sprite3.position.x = -32
# sprite3.scale.x = 0.5
# sprite3.scale.y = 0.5
# sprite.add_child(sprite3)

# sprite4 = MySprite()
# sprite4.position.y = 32
# sprite4.scale.x = 0.5
# sprite4.scale.y = 0.5
# sprite.add_child(sprite4)

# sprite5 = MySprite()
# sprite5.position.y = -32
# sprite5.scale.x = 0.5
# sprite5.scale.y = 0.5
# sprite.add_child(sprite5)

camera = CameraNode()

engine.start()


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