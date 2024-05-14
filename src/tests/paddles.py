import engine_main

import engine
import engine_debug
import engine_draw
import engine_io
import engine_physics
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Circle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode
import random

engine.set_fps_limit(30)

# Don't want gravity in pong, zero out the axes
engine_physics.set_gravity(0, 0)

# Create walls uses rectangle physics nodes and then add rectangle 2d
# nodes to render them, make them children of the physics nodes
top_wall_physics = PhysicsRectangle2DNode(width=128, height=5, bounciness=3, position=Vector2(0, -64), dynamic=False)
bot_wall_physics = PhysicsRectangle2DNode(width=128, height=5, bounciness=3, position=Vector2(0, 64), dynamic=False)

left_wall_physics = PhysicsRectangle2DNode(width=5, height=128, bounciness=3, position=Vector2(-64, 0), dynamic=False)
right_wall_physics = PhysicsRectangle2DNode(width=5, height=128, bounciness=3, position=Vector2(64, 0), dynamic=False)

top_wall_renderer = Rectangle2DNode(width=top_wall_physics.width, height=top_wall_physics.height, color=engine_draw.red, outline=True)
bot_wall_renderer = Rectangle2DNode(width=bot_wall_physics.width, height=bot_wall_physics.height, color=engine_draw.red, outline=True)
left_wall_renderer = Rectangle2DNode(width=left_wall_physics.width, height=left_wall_physics.height, color=engine_draw.red, outline=True)
right_wall_renderer = Rectangle2DNode(width=right_wall_physics.width, height=right_wall_physics.height, color=engine_draw.red, outline=True)

top_wall_physics.add_child(top_wall_renderer)
bot_wall_physics.add_child(bot_wall_renderer)
left_wall_physics.add_child(left_wall_renderer)
right_wall_physics.add_child(right_wall_renderer)

# Create the ball using a circle physics node
# and use a 2d circle node for rendering it
ball_physics = PhysicsCircle2DNode(bounciness=3)
ball = Circle2DNode(color=engine_draw.blue, radius=4, outline=True)
ball_physics.add_child(ball)

ball_physics.velocity.x = ((random.random() - 0.5) * 2.0) * 3
ball_physics.velocity.y = ((random.random() - 0.5) * 2.0) * 3


class PlayerPaddle(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.height = 20
        self.width = 5
        self.position = Vector2(-50, 0)
        self.bounciness=3
        self.paddle_rectangle = Rectangle2DNode(width=self.width, height=self.height, color=engine_draw.green, outline=True)

        self.add_child(self.paddle_rectangle)

        self.contact_anim_counter = 100
    
    def tick(self, dt):
        self.velocity.x = 0
        self.position.x = -50

        if self.contact_anim_counter < 100:
            self.paddle_rectangle.color = engine_draw.white
            self.contact_anim_counter = self.contact_anim_counter + 1
        else:
            self.paddle_rectangle.color = engine_draw.green

        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.velocity.y = -1.5
        elif engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.velocity.y = 1.5
        else:
            self.velocity.y = 0
    
    def collision(self, contact_point):
        # On contact with anything, flash color for a bit
        self.contact_anim_counter = 0


class AIPaddle(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.height = 20
        self.width = 5
        self.position = Vector2(50, 0)
        self.bounciness=3
        self.paddle_rectangle = Rectangle2DNode(width=self.width, height=self.height, color=engine_draw.yellow, outline=True)

        self.add_child(self.paddle_rectangle)

        self.contact_anim_counter = 100
    
    def tick(self, dt):
        self.velocity.x = 0
        self.position.x = 50
        self.color = engine_draw.yellow

        if self.contact_anim_counter < 100:
            self.paddle_rectangle.color = engine_draw.white
            self.contact_anim_counter = self.contact_anim_counter + 1
        else:
            self.paddle_rectangle.color = engine_draw.yellow

        if ball_physics.position.x > 0:
            if ball_physics.position.y < self.position.y - 8:
                self.velocity.y = -1.5
            elif ball_physics.position.y > self.position.y + 8:
                self.velocity.y = 1.5
        else:
            self.velocity.y = 0
    
    def collision(self, contact_point):
        # On contact with anything, flash color for a bit
        self.contact_anim_counter = 0


player = PlayerPaddle()
ai = AIPaddle()

camera = CameraNode()

# engine.start()
while True:
    engine.tick()