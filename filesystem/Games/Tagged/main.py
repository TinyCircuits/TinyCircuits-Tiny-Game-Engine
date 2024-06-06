import machine
#__freq = machine.freq()
#machine.freq(250000000)
import os
#os.chdir("/Games/Tagged")

import math

import engine_main

import random
import engine
import engine_io
import engine_physics
import engine_draw

from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource, NoiseResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN

import engine_debug

#machine.freq(__freq)

engine.set_fps_limit(120)

runner_texture = TextureResource("test_runner_frames_16bit.bmp")

cam = CameraNode()
player = Sprite2DNode()
#engine_physics.set_gravity(0.0, -0.2)

contact_point = PhysicsRectangle2DNode()
contact_point.outline = True

player_hitbox = PhysicsRectangle2DNode()

class PlayerHitbox(PhysicsRectangle2DNode):
    def __init__(self, player_spr):
        super().__init__(self)
        self.player_spr = player_spr
        self.outline = True
        self.width = 12
        self.height = 16
        self.add_child(player_spr)
    def tick(self, dt):
        if(self.velocity.x < 0):
            self.player_spr.position.x = -1.0
        else:
            self.player_spr.position.x = 1.0
        if(not self.player_spr.playing):
            self.player_spr.frame_current_x = 2
            self.player_spr.playing = True
    def collision(self, contact):
        contact_point.position = contact
        pass


player.texture = runner_texture
player.frame_count_x = 4
player.frame_current_x = 0
player.transparent_color = Color(0x07e0)
player.loop = False
player.playing = True
player.fps = 18
player.scale = Vector2(1.0, 1.0)
player_box = PlayerHitbox(player)
        
ground_box = PhysicsRectangle2DNode()
ground_box.position = Vector2(0, 64)
ground_box.width = 128
ground_box.height = 16
ground_box.outline = True
ground_box.dynamic = False

engine.start()
