import engine_main
import engine
import engine_io
import engine_draw

from engine_math import Vector2
from engine_nodes import PhysicsRectangle2DNode, PhysicsCircle2DNode, CameraNode

engine.set_fps_limit(60)

circle = PhysicsCircle2DNode(position=Vector2(0, -35), radius=7, outline=True, dynamic=False, collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0011)
ground = PhysicsRectangle2DNode(position=Vector2(0, 50), width=110, height=10, outline=True, dynamic=False, collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0011)
platform = PhysicsRectangle2DNode(position=Vector2(0, 15), width=30, height=5, outline=True, dynamic=False, collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0011)
agent = PhysicsRectangle2DNode(position=Vector2(0, 5), width=5, height=10, outline=True, collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0010)

class Ray(PhysicsRectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.dynamic = False
        self.solid = False
        self.outline = True
        self.outline_color = engine_draw.green
        self.width = 1
        self.collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0001

    def on_collide(self, contact):
        self.outline_color = engine_draw.red
    
    def on_separate(self):
        self.outline_color = engine_draw.green
    
class Sensor(PhysicsCircle2DNode):
    def __init__(self, position):
        super().__init__(self)
        self.dynamic = False
        self.solid = False
        self.outline = True
        self.outline_color = engine_draw.green
        self.radius = 3
        self.position = position
        self.collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0011

    def on_collide(self, contact):
        self.outline_color = engine_draw.red
    
    def on_separate(self):
        self.outline_color = engine_draw.green


ray = Ray()

class Player(PhysicsRectangle2DNode):
    def __init__(self, position):
        super().__init__(self)
        self.width=5
        self.height=10
        self.outline = True
        self.position = position
        self.gravity_scale.y = 0
        self.collision_layer=0b0000_0000_0000_0000_0000_0000_0000_0010

        self.rays = []
        self.sensors = []

        ray0 = Ray()
        ray0.adjust_from_to(Vector2(1, -7), Vector2(5, -30))
        self.add_child(ray0)
        self.rays.append(ray0)

        ray1 = Ray()
        ray1.adjust_from_to(Vector2(-1, -7), Vector2(-5, -30))
        self.add_child(ray1)
        self.rays.append(ray1)

        sen0 = Sensor(Vector2(-15, -25))
        self.add_child(sen0)
        self.sensors.append(sen0)

        sen1 = Sensor(Vector2(15, -25))
        self.add_child(sen1)
        self.sensors.append(sen1)
    
    def tick(self, dt):
        ray.adjust_from_to(self.position, agent.position)

        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.velocity.x = -1.0
        elif engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.velocity.x = 1.0
        else:
            self.velocity.x = 0.0
        
        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.velocity.y = -1.0
        elif engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.velocity.y = 1.0
        else:
            self.velocity.y = 0.0
        
        if engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            self.rotation -= 0.1
        elif engine_io.check_pressed(engine_io.BUMPER_LEFT):
            self.rotation += 0.1

player = Player(Vector2(0, 40))

cam = CameraNode()

engine.start()