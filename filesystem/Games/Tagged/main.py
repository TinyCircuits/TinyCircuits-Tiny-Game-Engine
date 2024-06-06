import machine
#__freq = machine.freq()
#machine.freq(250000000)
#import os
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
#engine_debug.enable_all()
#machine.freq(__freq)
engine.set_fps_limit(60)
runner_texture = TextureResource("test_runner_frames_16bit.bmp")
cam = CameraNode()
player = Sprite2DNode()
#engine_physics.set_gravity(0.0, -1.5)
#player_hitbox = PhysicsRectangle2DNode()
GROUND_ACCEL = 0.5
GROUND_DECEL = 1.2
GROUND_MAX = 2.4
AIR_ACCEL = 0.12
AIR_DECEL = 0.06
AIR_MAX = 6
class PlayerHitbox(PhysicsRectangle2DNode):
    def __init__(self, player_spr):
        super().__init__(self)
        self.player_spr = player_spr
        self.dynamic = True
        #self.solid = True
        #self.outline = True
        self.width = 12
        self.height = 16
        self.bounciness = 0.0
        self.friction = 0.0
        self.on_ground = True
        #self.density = 1.0
        self.add_child(player_spr)
    def physics_tick(self, dt):
        #print("Assuming player is midair...")
        if(self.on_ground):
            print("Player pre-ticked on ground!")
        else:
            print("Player pre-ticked midair!")
        if(engine_io.check_pressed(engine_io.DPAD_LEFT)):
            if(self.on_ground):
                self.velocity.x -= GROUND_ACCEL
                if(self.velocity.x < -GROUND_MAX):
                    self.velocity.x = -GROUND_MAX
            else:
                self.velocity.x -= AIR_ACCEL
                if(self.velocity.x < -AIR_MAX):
                    self.velocity.x = -AIR_MAX
        elif(self.velocity.x < 0):
            if(self.on_ground):
                self.velocity.x += GROUND_DECEL
                if(self.velocity.x > 0):
                    self.velocity.x = 0
            else:
                self.velocity.x += AIR_DECEL
                if(self.velocity.x > 0):
                    self.velocity.x = 0
        if(engine_io.check_pressed(engine_io.DPAD_RIGHT)):
            if(self.on_ground):
                print("Ground accel")
                self.velocity.x += GROUND_ACCEL
                if(self.velocity.x > GROUND_MAX):
                    self.velocity.x = GROUND_MAX
            else:
                print("Air accel")
                self.velocity.x += AIR_ACCEL
                if(self.velocity.x > AIR_MAX):
                    self.velocity.x = AIR_MAX
        elif(self.velocity.x > 0):
            if(self.on_ground):
                self.velocity.x -= GROUND_DECEL
                if(self.velocity.x < 0):
                    self.velocity.x = 0
            else:
                self.velocity.x -= AIR_DECEL
                if(self.velocity.x < 0):
                    self.velocity.x = 0
        if(engine_io.check_pressed(engine_io.A) and self.on_ground):
            self.velocity.y -= 0.6
        #if(self.on_ground):
        #    self.player_spr.frame_current_y = 0
        #else:
        #    self.player_spr.frame_current_y = 1
        #print("Physics pre-tick clearing ground flag")
        self.on_ground = False
    def tick(self, dt):
        if(self.velocity.x < 0):
            self.player_spr.scale.x = -1.0
        elif(self.velocity.x > 0):
            self.player_spr.scale.x = 1.0
        if(abs(self.velocity.x) < 0.1):
            self.player_spr.frame_current_x = 0
            self.player_spr.playing = False
        else:
            if(not self.player_spr.playing):
                self.player_spr.frame_current_x = 1
                self.player_spr.playing = True
        self.player_spr.frame_current_y = 0
    def collision(self, contact):
        #print("Contact at "+str(contact.position.x) + ", " + str(contact.position.y))
        #print(id(contact.node))
        #print(str(self.position.x) + ", " + str(self.position.y))
        #print(str(contact.node.position.x) + ", " + str(contact.node.position.y))
        self.on_ground = True
        print("Found the ground!")
        pass
player.texture = runner_texture
player.frame_count_x = 4
player.frame_current_x = 0
player.frame_count_y = 2
player.frame_current_y  = 0
player.transparent_color = Color(0x07e0)
player.loop = False
player.playing = True
player.fps = 18
player.scale = Vector2(1.0, 1.0)
player_box = PlayerHitbox(player)
ground_box = PhysicsRectangle2DNode(width=128, height=32, position=Vector2(0, 64), rotation=0, dynamic=False, bounciness = 1.)
ground_box.outline = True
ground_box.velocity = Vector2(0, 0)
ground_box.friction = 0.0
ground_box.bounciness = 0.0
#ground_box.gravity_scale = 0.0
#ground_box.density = 1000000
#engine.start()
while(True):
    if(engine.tick()):
        print("Ticking")
        #if(not player.playing):
            #player.frame_current_x = 2
            #player.playing = True