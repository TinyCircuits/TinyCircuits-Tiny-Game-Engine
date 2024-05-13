import engine_main

import engine
import engine_draw
import engine_io
import engine_physics
import math
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Circle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode

engine.set_fps_limit(60)
engine_physics.set_gravity(0, 0)


ball_dia_mm = 57.15 # https://images.app.goo.gl/sfgWqEnEr52cXHUD8
hole_dia_mm = 133   # https://images.app.goo.gl/YDCsrbHLMnVDQ7eu7

# https://images.app.goo.gl/nDPWRLb662uDcdA2A
table_inside_len_mm = 2240
table_inside_wid_mm = 1120
table_wall_thickness_mm = (1500 - 1120) / 2

stick_len_mm = 1447.8
stick_dia_mm = 25.4


class Border(PhysicsRectangle2DNode):
    def __init__(self, direction, length, position):
        super().__init__(self)

        if(direction == "h"):
            self.width = length
            self.height = table_wall_thickness_mm
        elif(direction == "v"):
            self.width = table_wall_thickness_mm
            self.height = length
        
        self.position = position
        self.outline = True
        self.dynamic = False


class PowerIndicator(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)

        self.width = 36
        self.height = 4
        self.position.y = 64 - self.height/2 - 2
        self.position.x = -64 + self.width/2 + 1
        self.outline = True
        self.color = engine_draw.blue
        self.set_layer(1)

        self.bar = Rectangle2DNode()
        self.bar.width = self.width
        self.bar.height = self.height
        self.bar.position.y = 0
        self.bar.position.x = 1
        self.bar.color = engine_draw.green
        self.bar.set_layer(0)

        self.add_child(self.bar)

        self.set_percent(0)
    
    def set_percent(self, percent):
        self.bar.width = (self.width) * percent
        self.bar.position.x = 1 - self.width/2 + self.bar.width/2


class Ball(PhysicsCircle2DNode):
    def __init__(self, power_indicator):
        super().__init__(self)

        self.power_indicator = power_indicator

        self.outline = True
        self.radius = ball_dia_mm/2
        self.position.y = table_inside_len_mm/3

        self.stick = Rectangle2DNode(outline=True)
        self.stick.width = stick_dia_mm
        self.stick.height = stick_len_mm
        self.stick.position.y = self.stick.height/2 + 25.4*4
        self.stick.color = engine_draw.brown

        self.add_child(self.stick)

        self.power = 0
        self.max_power = 10
    
    def tick(self, dt):
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            self.rotation -= 0.05
        elif engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            self.rotation += 0.05
        
        if engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.power += 0.25
            print(self.power)

            if self.power > self.max_power:
                self.power = self.max_power
        
        if engine_io.check_just_released(engine_io.DPAD_DOWN):
            self.velocity.x = -math.cos(self.rotation - math.pi/2) * self.power
            self.velocity.y = math.sin(self.rotation - math.pi/2) * self.power

            self.power = 0
            print("Shot!")

            print(self.velocity.x, self.velocity.y)
        
        percent = self.power / self.max_power
        self.power_indicator.set_percent(self.power / self.max_power)
        # self.stick.position.y = self.stick.height/2 + (25.4*12 * percent)


class GameCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self, dt):
        if engine_io.check_pressed(engine_io.A):
            self.zoom += 0.005
        elif engine_io.check_pressed(engine_io.B):
            self.zoom -= 0.005


camera = GameCamera()
camera.zoom = 0.045

power_indicator = PowerIndicator()
camera.add_child(power_indicator)

top_border = Border("h", table_inside_wid_mm-(hole_dia_mm*2), Vector2(0,   table_inside_len_mm/2 + table_wall_thickness_mm/2))
bot_border = Border("h", table_inside_wid_mm-(hole_dia_mm*2), Vector2(0, -(table_inside_len_mm/2 + table_wall_thickness_mm/2)))

left_border = Border("v", table_inside_len_mm-(hole_dia_mm*2), Vector2(-(table_inside_wid_mm/2 + table_wall_thickness_mm/2), 0))
right_border = Border("v", table_inside_len_mm-(hole_dia_mm*2), Vector2(table_inside_wid_mm/2 + table_wall_thickness_mm/2, 0))

ball = Ball(power_indicator)
ball.add_child(camera)


balls = []
x = ball_dia_mm/2 - ball_dia_mm - ball_dia_mm/2 - 2

for i in range(5):
    b = PhysicsCircle2DNode()
    b.outline = True
    b.radius = ball_dia_mm/2
    b.position.x = x
    x += ball_dia_mm + 1
    balls.append(b)


engine.start()