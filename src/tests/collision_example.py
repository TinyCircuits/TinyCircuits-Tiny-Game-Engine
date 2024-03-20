import engine
import engine_draw
import engine_input
import engine_physics
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Circle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode
import random
import math

# Imagine we're targeting a top-down game, turn gravity off on each axis
# engine_physics.set_gravity(0, 0)


circle_physics_0 = PhysicsCircle2DNode(position=Vector2(0, -15), radius=8)
circle_0 = Circle2DNode(outline=True, radius=8)
circle_physics_0.add_child(circle_0)


box_physics_0 = PhysicsRectangle2DNode(width=20, height=20, position=Vector2(25, -35), rotation=0, dynamic=False, bounciness = 2.0)
box_0 = Rectangle2DNode(width=20, height=20, outline=True)
box_0.color = engine_draw.blue
box_physics_0.add_child(box_0)
 

floor_box_physics = PhysicsRectangle2DNode(width=128, height=10, position=Vector2(0, 64), dynamic=False)
floor_box = Rectangle2DNode(width=128, height=10, color=engine_draw.green, outline=True)
floor_box_physics.add_child(floor_box)


boxes = []
for i in range(6):
    stacked_box_physics = PhysicsRectangle2DNode(width=7, height=7, position=Vector2(0, i*7))
    stacked_box = Rectangle2DNode(width=7, height=7, color=engine_draw.green, outline=True)
    stacked_box_physics.add_child(stacked_box)
    boxes.append(stacked_box_physics)
    boxes.append(stacked_box)

class Player(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector2(0, -50)

        self.bounciness = 2.0

        # self.radius = 7

        self.player = Rectangle2DNode(color=engine_draw.green, outline=True)
        self.add_child(self.player)

        self.count = 0
        print(self.count)
    
    def tick(self):
        if engine_input.check_pressed(engine_input.DPAD_LEFT):
            self.velocity.x = -0.55
        elif engine_input.check_pressed(engine_input.DPAD_RIGHT):
            self.velocity.x = 0.55

        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.velocity.y = -0.55
        elif engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.velocity.y = 0.55

        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation += 0.0045
        elif engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation -= 0.0045
    
    def collision(self, contact):
        self.count = self.count + 1
        print("Collision!", self.count)
        # Circle2DNode(position=contact.position, radius=2)



player = Player()


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
    
    def tick(self):
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


gate = Gate()
gate.position.y = 30

box_physics_1 = rect()
box_1 = Rectangle2DNode(width=15, height=15, outline=True)
box_1.color = engine_draw.blue
box_physics_1.add_child(box_1)


camera = CameraNode()

engine.start()
