import engine_main

import engine
import engine_draw
import engine_io
import engine_physics
import math
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Circle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode

engine.fps_limit(60)
engine_physics.set_gravity(0, 0)


ball_dia_mm = 57.15 # https://images.app.goo.gl/sfgWqEnEr52cXHUD8
ball_dia_with_margin_mm = ball_dia_mm + 5
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
    def __init__(self):
        super().__init__(self)

        self.outline = True
        self.radius = ball_dia_mm/2

    def tick(self, dt):
        vel_length = self.velocity.length()
        if vel_length > 0.5:
            normal = self.velocity.normalized()
            self.velocity.x += -normal.x * 0.07
            self.velocity.y += -normal.y * 0.07
        elif vel_length > 0.0 and vel_length <= 0.5:
            self.velocity.x = 0
            self.velocity.y = 0


class Stick(Rectangle2DNode):
    def __init__(self, ball, power_indicator):
        super().__init__(self)

        self.ball = ball
        self.ball.add_child(self)

        self.power_indicator = power_indicator

        self.power = 0
        self.max_power = 35

        self.outline = True
        self.width = stick_dia_mm
        self.height = stick_len_mm
        self.position.y = self.height/2 + 25.4*4
        self.color = engine_draw.brown

    def tick(self, dt):
        # Rotation
        if engine_io.LB.is_pressed:
            self.ball.rotation -= 0.025
        elif engine_io.RB.is_pressed:
            self.ball.rotation += 0.025

        # Power
        if engine_io.DOWN.is_pressed:
            self.power += 1

            if self.power > self.max_power:
                self.power = self.max_power

        percent = self.power / self.max_power
        self.power_indicator.set_percent(self.power / self.max_power)

        # Shoot
        if engine_io.DOWN.is_just_released:
            self.ball.velocity.x = -math.cos(self.ball.rotation - math.pi/2) * self.power
            self.ball.velocity.y = math.sin(self.ball.rotation - math.pi/2) * self.power
            self.power = 0


class GameCamera(CameraNode):
    def __init__(self):
        super().__init__(self)

    def tick(self, dt):
        if engine_io.A.is_pressed:
            self.zoom += 0.005
        elif engine_io.B.is_pressed:
            self.zoom -= 0.005


camera = GameCamera()
camera.zoom = 0.045

power_indicator = PowerIndicator()
camera.add_child(power_indicator)

top_border = Border("h", table_inside_wid_mm-(hole_dia_mm*2), Vector2(0,   table_inside_len_mm/2 + table_wall_thickness_mm/2))
bot_border = Border("h", table_inside_wid_mm-(hole_dia_mm*2), Vector2(0, -(table_inside_len_mm/2 + table_wall_thickness_mm/2)))

left_border = Border("v", table_inside_len_mm-(hole_dia_mm*2), Vector2(-(table_inside_wid_mm/2 + table_wall_thickness_mm/2), 0))
right_border = Border("v", table_inside_len_mm-(hole_dia_mm*2), Vector2(table_inside_wid_mm/2 + table_wall_thickness_mm/2, 0))

ball = Ball()
ball.add_child(camera)
ball.position.y = table_inside_len_mm/3

stick = Stick(ball, power_indicator)

balls = []

def rack_row(ball_count, x, y):
    for i in range(ball_count):
        b = Ball()
        b.outline = True
        b.radius = ball_dia_with_margin_mm/2
        b.position.x = x
        b.position.y = y
        x += ball_dia_with_margin_mm
        balls.append(b)

rack_row(5, -ball_dia_with_margin_mm/2 - ball_dia_with_margin_mm - ball_dia_with_margin_mm/2, 0)
rack_row(4, -ball_dia_with_margin_mm/2 - ball_dia_with_margin_mm, ball_dia_with_margin_mm)
rack_row(3, -ball_dia_with_margin_mm, ball_dia_with_margin_mm*2)
rack_row(2, -ball_dia_with_margin_mm/2, ball_dia_with_margin_mm*3)
rack_row(1, 0, ball_dia_with_margin_mm*4)

engine.start()