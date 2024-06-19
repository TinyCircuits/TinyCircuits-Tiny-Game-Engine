import engine_main

import engine
import engine_draw
import engine_debug
import engine_io
import engine_physics
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, VoxelSpaceSpriteNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import gc
import math
import os
import time

os.chdir("Games/VoxelSpaceExample")

# import machine

# machine.freq(250 * 1000 * 1000)

engine_io.gui_toggle_button = None

engine.set_fps_limit(60)
engine_draw.set_background_color(engine_draw.skyblue)


C18W = TextureResource("C18W.bmp", True)
D18 = TextureResource("D18.bmp", True)
tree_bmp = TextureResource("tree.bmp", True)
tc_bmp = TextureResource("32x32.bmp", True)

vox0 = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox0.position.x = 0
vox0.position.y = 30
vox0.scale.y = 35
vox0.scale.x = 2
vox0.scale.z = 2
vox0.flip = True
vox0.repeat = True



vox1 = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox1.position.x = 0
vox1.position.y = 0
vox1.position.z = 0
vox1.scale.y = 32
vox1.scale.x = 0.75
vox1.scale.z = 0.75
vox1.flip = False
vox1.thickness = 100
# vox1.repeat = True
# vox1.set_layer(1)

tree = VoxelSpaceSpriteNode(texture=tree_bmp, position=Vector3(0, 0, 0), scale=Vector3(1.0, 1.0, 1.0))
tree.transparent_color = engine_draw.white
tree.opacity = 1.0
tree.position.x = 75
tree.position.z = 75
tree.position.y = vox1.get_abs_height(tree.position.x, tree.position.z)
tree.scale.x = 0.1
tree.scale.y = 0.1
tree.texture_offset.y = tree_bmp.height/2
tree.rotation = math.pi
# tree.texture_offset.x = tree_bmp.width/2
tree.fov_distort = False
# # tree.set_layer(0)


bmp = VoxelSpaceSpriteNode(texture=tc_bmp, position=Vector3(0, 0, 0), scale=Vector3(1.0, 1.0, 1.0))
# bmp.transparent_color = engine_draw.black
bmp.opacity = 1.0
bmp.position.x = 32
bmp.position.z = 16
bmp.position.y = vox1.get_abs_height(bmp.position.x, bmp.position.z)



class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.25
        self.mode = 0
        self.t = 0

    def adjust(self):
        vox0_height = vox0.get_abs_height(self.position.x, self.position.z)
        vox1_height = vox1.get_abs_height(self.position.x, self.position.z)

        if(vox1_height != None and self.position.y < vox1_height):
            self.position.y = vox1_height
        
        if(vox0_height != None and self.position.y > vox0_height):
            self.position.y = vox0_height

    def forward(self):
        self.position.x += math.cos(self.rotation.y) * self.distance
        self.position.z += math.sin(self.rotation.y) * self.distance
        self.adjust()            
    
    def backward(self):
        self.position.x -= math.cos(self.rotation.y) * self.distance
        self.position.z -= math.sin(self.rotation.y) * self.distance
        self.adjust() 
    
    def left(self):
        self.position.x -= math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z -= math.sin(self.rotation.y+(math.pi/2)) * self.distance
        self.adjust() 
    
    def right(self):
        self.position.x += math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z += math.sin(self.rotation.y+(math.pi/2)) * self.distance
        self.adjust()

    def tick(self, dt):
        gc.collect()
        print(self.position)
        # print(engine.get_running_fps())
        self.t += 0.01
        # vox0.position.y = 20 + math.sin(self.t)

        if engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            if self.mode == 0:
                self.rotation.y += 0.025
            elif self.mode == 2:
                self.rotation.z += 0.005
                # self.rotation.x -= 0.005
            elif self.mode == 3:
                vox0.lod += 0.001
                vox1.lod += 0.001
            elif self.mode == 4:
                vox0.curvature += 0.005
                vox1.curvature += 0.005
            elif self.mode == 5:
                vox0.scale.y += 1
                vox1.scale.y += 1
            elif self.mode == 6:
                self.fov += 0.05
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            if self.mode == 0:
                self.rotation.y -= 0.025
            elif self.mode == 2:
                self.rotation.z -= 0.005
                self.rotation.x += 0.005
            elif self.mode == 3:
                vox0.lod -= 0.001
                vox1.lod -= 0.001
            elif self.mode == 4:
                vox0.curvature -= 0.005
                vox1.curvature -= 0.005
            elif self.mode == 5:
                vox0.scale.y -= 1
                vox1.scale.y -= 1
            elif self.mode == 6:
                self.fov -= 0.05
    

        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.forward()
        if engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.backward()
        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.left()
        if engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.right()
        
        if engine_io.check_pressed(engine_io.A):
            if self.mode == 0:
                self.position.y += 0.1
                self.adjust() 
            elif self.mode == 1:
                self.rotation.x -= 0.1
        if engine_io.check_pressed(engine_io.B):
            if self.mode == 0:
                self.position.y -= 0.1
                self.adjust() 
            elif self.mode == 1:
                self.rotation.x += 0.1
        
        if engine_io.check_just_pressed(engine_io.MENU):
            # vox.scale.y += 0.5
            # print(vox.scale.y)
            self.mode = self.mode + 1
            if self.mode >= 7:
                self.mode = 0
            
            print(self.mode)


camera = MyCam()
camera.position.x = 0
camera.position.y = 0
camera.position.z = 0
# camera.rotation.x = 0.3
camera.view_distance = 350
# camera.fov = 32 * (math.pi/180)
camera.fov = 70 * (math.pi/180)

camera.add_child(Circle2DNode(radius=1, color=engine_draw.green, outline=True))

engine.start()