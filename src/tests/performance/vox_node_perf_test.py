import engine_main

import engine
import engine_draw
from engine_nodes import VoxelSpaceNode, CameraNode
from engine_resources import TextureResource
import math

engine.disable_fps_limit()

engine_draw.set_background_color(engine_draw.skyblue)

C18W = TextureResource("C18W.bmp", True)
D18 = TextureResource("D18.bmp", True)

vox = VoxelSpaceNode(texture=C18W, heightmap=D18)
vox.position.x = 200
vox.position.y = 0
vox.scale.y = 10


camera = CameraNode()
camera.position.x = 175
camera.position.y = 10
camera.position.z = 75
camera.view_distance = 350
camera.fov = 70 * (math.pi/180)

ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[vox_node_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")

import machine
machine.soft_reset()