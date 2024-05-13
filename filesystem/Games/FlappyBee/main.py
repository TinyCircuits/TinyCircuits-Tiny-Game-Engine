import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_math import Vector2
import os

font = FontResource("outrunner_outline.bmp")
texture = TextureResource("bee.bmp")

engine.set_fps_limit(60)
engine_physics.set_gravity(0, -0.02)

collided = False
youre_bad = None

class Bee(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.bee = Sprite2DNode(texture=texture, frame_count_x=2)
        self.width = 32
        self.height = 32

        self.add_child(self.bee)
    
    def collision(self, contact):
        global collided
        global youre_bad
        collided = True
        
        youre_bad = Text2DNode(font=font, text="You're bad!", scale=Vector2(1.25, 1.25))

        self.velocity = Vector2(0, 0)


bird = Bee()
camera = CameraNode()


class Obstacle(EmptyNode):
    def __init__(self):
        super().__init__(self)
        self.top_physics = PhysicsRectangle2DNode(width=8, gravity_scale=Vector2(1.0, 0.0), velocity=Vector2(-1, 0))
        self.bot_physics = PhysicsRectangle2DNode(width=8, gravity_scale=Vector2(1.0, 0.0), velocity=Vector2(-1, 0))

        self.top = Rectangle2DNode(color=engine_draw.yellow, width=8)
        self.bot = Rectangle2DNode(color=engine_draw.yellow, width=8)

        self.top_physics.add_child(self.top)
        self.bot_physics.add_child(self.bot)

        self.random_size()
    
    def tick(self, dt):
        if self.top_physics.position.x < -64 - self.top_physics.width/2:
            self.top_physics.position.x = 64 + self.top_physics.width/2
            self.bot_physics.position.x = 64 + self.top_physics.width/2
            self.random_size()
        
        global collided
        if collided:
            self.top_physics.velocity = Vector2(0, 0)
            self.bot_physics.velocity = Vector2(0, 0)
            self.top_physics.dynamic = False
            self.bot_physics.dynamic = False
    
    def random_size(self):
        opening_height = random.uniform(50, 110)
        total_height = 128-opening_height

        top_percentage = random.uniform(0.1, 1.0)
        bot_percentage = 1.0 - top_percentage

        self.top_physics.height = total_height * top_percentage
        self.bot_physics.height = total_height * bot_percentage

        self.top_physics.position.y =  64 - self.top_physics.height/2
        self.bot_physics.position.y = -64 + self.bot_physics.height/2

        self.top.height = self.top_physics.height
        self.bot.height = self.bot_physics.height
    
    def set_x(self, x):
        self.top_physics.position.x = x
        self.bot_physics.position.x = x

ob0 = Obstacle()
ob0.set_x(64)

ob1 = Obstacle()
ob1.set_x(64+64)

tick = 0

while tick < 180:

    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():
        if engine_io.check_pressed(engine_io.A):
            bird.velocity.y -= 0.2

        if collided:
            tick += 1