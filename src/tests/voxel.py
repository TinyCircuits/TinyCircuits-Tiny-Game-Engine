import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_physics import PhysicsShapeRectangle
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import gc
import math
import os
import time

engine_debug.debug_enable_setting(engine_debug.debug_setting_performance)


engine_draw.set_background_color(engine_draw.skyblue)


C18W = TextureResource("C14W.bmp")
D18 = TextureResource("D14.bmp")

vox = VoxelSpaceNode(C18W, D18)

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.1
        self.mode = 0

    def forward(self):
        self.position.x -= math.cos(self.rotation.y) * self.distance
        self.position.z -= math.sin(self.rotation.y) * self.distance
    
    def backward(self):
        self.position.x += math.cos(self.rotation.y) * self.distance
        self.position.z += math.sin(self.rotation.y) * self.distance
    
    def left(self):
        self.position.x += math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z += math.sin(self.rotation.y+(math.pi/2)) * self.distance
    
    def right(self):
        self.position.x -= math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z -= math.sin(self.rotation.y+(math.pi/2)) * self.distance

    def tick(self):
        if engine_input.is_bumper_right_pressed():
            self.rotation.y += 0.05
        if engine_input.is_bumper_left_pressed():
            self.rotation.y -= 0.05
    

        if engine_input.is_dpad_up_pressed():
            self.forward()
        if engine_input.is_dpad_down_pressed():
            self.backward()
        if engine_input.is_dpad_left_pressed():
            self.left()
        if engine_input.is_dpad_right_pressed():
            self.right()
        
        if engine_input.is_a_pressed():
            if self.mode == 0:
                self.position.y -= 1
            else:
                self.rotation.x -= 0.5
        if engine_input.is_b_pressed():
            if self.mode == 0:
                self.position.y += 1
            else:
                self.rotation.x += 0.5
        
        if engine_input.is_menu_pressed():
            if self.mode == 0:
                self.mode = 1
            else:
                self.mode = 0


camera = MyCam()
camera.position.x = 75
camera.position.y = 50
camera.position.z = 75
engine.start()