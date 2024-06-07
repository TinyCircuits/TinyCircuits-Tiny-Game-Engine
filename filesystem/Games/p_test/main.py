import machine
#__freq = machine.freq()
#machine.freq(250000000)
import os
os.chdir("/Games/Collision2")

import math
import urandom
import gc

import engine_main

import random
import engine
import engine_io
import engine_physics
import engine_draw

from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, PhysicsCircle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource, NoiseResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN

import engine_debug

#engine_debug.enable_all()

#machine.freq(__freq)

engine.set_fps_limit(60)

cam = CameraNode()
engine_physics.set_gravity(0.0, -0.1)
        
ground_box = PhysicsRectangle2DNode(width=128, height=16, position=Vector2(0, 64), rotation=0, dynamic=False, bounciness = 1.)
ground_box.outline = True
ground_box.velocity = Vector2(0, 0)
ground_box.friction = 0.0
ground_box.bounciness = 0.1
ground_box.rotation = 0.0

wall_boxes = [PhysicsRectangle2DNode(), PhysicsRectangle2DNode()]
wall_boxes[0].outline = True
wall_boxes[1].outline = True
wall_boxes[0].position = Vector2(-64, 0)
wall_boxes[1].position = Vector2(64, 0)

wall_boxes[0].width = 16
wall_boxes[1].width = 16

wall_boxes[0].height = 128
wall_boxes[1].height = 128

wall_boxes[0].dynamic = False
wall_boxes[1].dynamic = False

wall_boxes[0].friction = 0.0
wall_boxes[0].bounciness = 0.1
wall_boxes[0].rotation = 0.0

wall_boxes[1].friction = 0.0
wall_boxes[1].bounciness = 0.1
wall_boxes[1].rotation = 0.0
#ground_box.rotation = 0.0
#ground_box.gravity_scale = 0.0
#ground_box.density = 1000000
'''
platform_box = PhysicsRectangle2DNode(width=48, height=16, position=Vector2(0, 16), rotation=0, dynamic=False, bounciness = 1.)
platform_box.outline = True
platform_box.velocity = Vector2(0, 0)
platform_box.friction = 0.0
platform_box.bounciness = 0.1
platform_box.rotation = math.pi/16
'''

BOXES_N = 2

boxes = [None] * BOXES_N

def collision(box, contact):
    if(contact.normal.x == 0 and contact.normal.y == 0):
        print("BAD NORMAL VECTOR!!!!!")

for i in range(BOXES_N):
    rect = PhysicsCircle2DNode(position=Vector2(urandom.randrange(-32, 32), urandom.randrange(-32, 32)))
    
    rect.friction = 0.0
    rect.bounciness = 0.3
    rect.outline = True
    rect.collision = collision
    boxes[i] = rect
    
#def platform_tick(plat, dt):
#    plat.rotation += dt
    
#platform_box.tick = platform_tick


#engine.start()

while(True):
    if(engine.tick()):
        #print(engine.get_running_fps())
        #print(gc.mem_free())
        for b in boxes:
            print(b.position)
        pass
        #print("Ticking")
        #if(not player.playing):
            #player.frame_current_x = 2
            #player.playing = True