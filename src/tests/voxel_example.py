import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import gc
import math
import os
import time

engine.set_fps_limit(60)
engine_draw.set_background_color(engine_draw.skyblue)


C18W = TextureResource("C18W.bmp", True)
D18 = TextureResource("D18.bmp", True)

vox = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox.position.x = 200
vox.position.y = -50
vox.height_scale = 40

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.mode = 0
        self.intensity = 0

    def forward(self):
        self.position.x += math.cos(self.rotation.y) * self.distance
        self.position.z += math.sin(self.rotation.y) * self.distance
    
    def backward(self):
        self.position.x -= math.cos(self.rotation.y) * self.distance
        self.position.z -= math.sin(self.rotation.y) * self.distance
    
    def left(self):
        self.position.x -= math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z -= math.sin(self.rotation.y+(math.pi/2)) * self.distance
    
    def right(self):
        self.position.x += math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z += math.sin(self.rotation.y+(math.pi/2)) * self.distance

    def tick(self):
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation.y += 0.05
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation.y -= 0.05
    

        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.intensity += 0.005
            engine_input.rumble(self.intensity)
            self.forward()
        if engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.intensity -= 0.005
            engine_input.rumble(self.intensity)
            self.backward()
        if engine_input.check_pressed(engine_input.DPAD_LEFT):
            self.left()
        if engine_input.check_pressed(engine_input.DPAD_RIGHT):
            self.right()
        
        if engine_input.check_pressed(engine_input.A):
            if self.mode == 0:
                self.position.y -= 1
            else:
                self.rotation.x -= 0.5
        if engine_input.check_pressed(engine_input.B):
            if self.mode == 0:
                self.position.y += 1
            else:
                self.rotation.x += 0.5
        
        if engine_input.check_pressed(engine_input.MENU):
            if self.mode == 0:
                self.mode = 1
            else:
                self.mode = 0


camera = MyCam()
camera.position.x = 175
camera.position.y = 200
camera.position.z = 175
camera.view_distance = 350
camera.fov = 70 * (math.pi/180)

camera.add_child(Circle2DNode(radius=3, color=engine_draw.green, outline=True))

engine.start()