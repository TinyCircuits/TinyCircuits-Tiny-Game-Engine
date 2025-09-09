import engine_main

import engine
import engine_draw
import engine_io
import engine_physics
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Circle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode
import random
import math


engine.fps_limit(50)


# Imagine we're targeting a top-down game, turn gravity off on each axis
# engine_physics.set_gravity(0, 0)


# circle_physics_0 = PhysicsCircle2DNode(position=Vector2(0, -15), radius=8)
# circle_0 = Circle2DNode(outline=True, radius=8)
# circle_physics_0.add_child(circle_0)


box_physics_0 = PhysicsRectangle2DNode(width=20, height=20, position=Vector2(25, -35), rotation=0, dynamic=False, bounciness = 1.0)
box_0 = Rectangle2DNode(width=20, height=20, outline=True)
box_0.color = engine_draw.blue
box_physics_0.add_child(box_0)


floor_box_physics = PhysicsRectangle2DNode(width=128, height=10, position=Vector2(0, 64), dynamic=False, bounciness=0.5)
floor_box = Rectangle2DNode(width=128, height=10, color=engine_draw.green, outline=True)
floor_box_physics.add_child(floor_box)


boxes = []
for i in range(6):
    stacked_box_physics = PhysicsRectangle2DNode(width=7, height=7, position=Vector2(0, -20+i*9))
    stacked_box = Rectangle2DNode(width=7, height=7, color=engine_draw.green, outline=True)
    stacked_box_physics.add_child(stacked_box)
    boxes.append(stacked_box_physics)
    boxes.append(stacked_box)
    stacked_box_physics.velocity.y = -0.5
    stacked_box_physics.bounciness = 0.0
    stacked_box_physics.density = 0.01
    # stacked_box_physics.angular_velocity = 0.1

class Player(PhysicsCircle2DNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector2(5, -50)

        self.bounciness = 0.5
        # self.width=20
        # self.height=20
        self.density = 0.01

        # self.radius = 7

        self.player = Circle2DNode(color=engine_draw.green, outline=True)
        self.add_child(self.player)

        self.count = 0
        print(self.count)

    def tick(self, dt):
        if engine_io.LEFT.is_pressed:
            self.velocity.x = -0.3
        elif engine_io.RIGHT.is_pressed:
            self.velocity.x = 0.3

        if engine_io.UP.is_pressed:
            self.velocity.y = -0.3
        elif engine_io.DOWN.is_pressed:
            self.velocity.y = 0.3

        if engine_io.LB.is_pressed:
            self.rotation += 0.0045
        elif engine_io.RB.is_pressed:
            self.rotation -= 0.0045

    def on_collide(self, contact):
        self.count = self.count + 1
        print("Collision!", self.count)
        Circle2DNode(position=contact.position, radius=1)

camera = CameraNode()

player = Player()
player.add_child(camera)


# circle_physics_1 = PhysicsCircle2DNode(radius=10, position=Vector2(20, 15), dynamic=False)
# circle_1 = Circle2DNode(outline=True, radius=10)
# circle_physics_1.add_child(circle_1)


class rect(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector2(-20, -30)

        self.width=15
        self.height=15
        self.rotation=math.pi/4
        self.dynamic=False

    def tick(self, dt):
        self.rotation -= 0.0025


class Gate(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.width = 30
        self.height = 3
        self.dynamic = False

        self.gate = Rectangle2DNode(width=30, height=3, color=engine_draw.red)
        self.add_child(self.gate)

    def collision(self, contact):
        if self.position.x > -40:
            self.position.x = self.position.x - 0.05


# gate = Gate()
# gate.position.y = 30

box_physics_1 = rect()
box_1 = Rectangle2DNode(width=15, height=15, outline=True)
box_1.color = engine_draw.blue
box_physics_1.add_child(box_1)




engine.start()