import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
import engine_audio
from engine_physics import EmptyPolyCollisionShape2D, RectanglePolyCollisionShape2D, HexagonPolyCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Polygon2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource, WaveSoundResource
import math
import gc

# engine_debug.enable_all()
# engine_debug.enable_setting(engine_debug.performance)


pool_ball_radius = 2.25 / 2

pool_table = Rectangle2DNode()
pool_table.width = 44
pool_table.height = 88
pool_table.outline = True


class PlayBall(Circle2DNode):
    def __init__(self, pos, radius, outline):
        # Due to MicroPython limitations, cannot use keyword arguments in direct call to __init__
        # https://github.com/orgs/micropython/discussions/10236#discussioncomment-4415329
        # https://github.com/micropython/micropython/blob/2bdaa1bedede63ee2380aa5de67802cb5d2cfcd1/py/objtype.c#L91
        super().__init__(self, pos, radius, engine_draw.white, True)
        self.hitting = False
    
    def tick(self):
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation += 0.01
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation -= 0.01
        
        if engine_input.check_pressed(engine_input.A) or engine_input.check_pressed(engine_input.B):
            self.hitting = True
            self.get_child(0).position.y = 10


pocket_top_left = Circle2DNode(Vector2(-pool_table.width/2, -pool_table.height/2), pool_ball_radius * 1.25, engine_draw.red, True)
pocket_top_right = Circle2DNode(Vector2(pool_table.width/2, -pool_table.height/2), pool_ball_radius * 1.25, engine_draw.red, True)
pocket_bottom_left = Circle2DNode(Vector2(-pool_table.width/2, pool_table.height/2), pool_ball_radius * 1.25, engine_draw.red, True)
pocket_bottom_right = Circle2DNode(Vector2(pool_table.width/2, pool_table.height/2), pool_ball_radius * 1.25, engine_draw.red, True)
pocket_middle_left = Circle2DNode(Vector2(-pool_table.width/2, 0), pool_ball_radius * 1.25, engine_draw.red, True)
pocket_middle_right = Circle2DNode(Vector2(pool_table.width/2, 0), pool_ball_radius * 1.25, engine_draw.red, True)

# Setup the pool balls for a break
pool_balls = []
balls_this_row = 5
ball_row_x_start = 0
ball_row_y_start = 0
pool_ball_radius_margin = pool_ball_radius * 1.05
while balls_this_row > 0:

    for i in range(balls_this_row):
        new_ball = Circle2DNode(Vector2(ball_row_x_start + (i*pool_ball_radius_margin*2), ball_row_y_start), pool_ball_radius, engine_draw.purple, True)
        new_ball.position.x -= pool_ball_radius_margin*4
        new_ball.position.y -= pool_table.height/2 * (2/3)
        pool_balls.append(new_ball)

    balls_this_row -= 1
    ball_row_x_start += pool_ball_radius_margin
    ball_row_y_start += pool_ball_radius_margin*2


camera = CameraNode()


pool_stick = Rectangle2DNode()
pool_stick.width = 2
pool_stick.height = 50
pool_stick.position = Vector2(0, pool_stick.height/2+pool_ball_radius+7)
pool_stick.color = engine_draw.brown

white_ball = PlayBall(Vector2(0, pool_table.height/2 * (2/3)), pool_ball_radius, True)
white_ball.add_child(pool_stick)
white_ball.add_child(camera)

engine.start()