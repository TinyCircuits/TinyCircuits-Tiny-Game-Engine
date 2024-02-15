import engine
import engine_draw
import engine_input
import engine_physics
import engine_utilities
from engine_math import Vector2
from engine_physics import PolygonCollisionShape2D
from engine_nodes import Polygon2DNode, Rectangle2DNode, Circle2DNode, CameraNode, Physics2DNode

engine_physics.set_gravity(0, 0)

camera = CameraNode()

area_wall_thickness = 5
area_wall_thickness_half = area_wall_thickness/2

arena_horizontal_poly_verts = [ Vector2(-camera.viewport.width/2,   -area_wall_thickness_half),
                                Vector2( camera.viewport.width/2-1, -area_wall_thickness_half),
                                Vector2( camera.viewport.width/2-1,  area_wall_thickness_half),
                                Vector2(-camera.viewport.width/2,    area_wall_thickness_half)]

arena_vertical_poly_verts =   [ Vector2(-area_wall_thickness_half,   -camera.viewport.height/2),
                                Vector2( area_wall_thickness_half,   -camera.viewport.height/2),
                                Vector2( area_wall_thickness_half,    camera.viewport.height/2-1),
                                Vector2(-area_wall_thickness_half,    camera.viewport.height/2-1)]

arena_horizontal_wall_shape = PolygonCollisionShape2D(vertices=arena_horizontal_poly_verts)
arena_vertical_wall_shape = PolygonCollisionShape2D(vertices=arena_vertical_poly_verts)


arena_top_wall_physics = Physics2DNode(position=Vector2(0, -camera.viewport.height/2), collision_shape=arena_horizontal_wall_shape, dynamic=False, bounciness=2)
arena_top_wall = Rectangle2DNode(width=camera.viewport.width, height=area_wall_thickness, color=engine_draw.red)
arena_top_wall_physics.add_child(arena_top_wall)

arena_bottom_wall_physics = Physics2DNode(position=Vector2(0, camera.viewport.height/2), collision_shape=arena_horizontal_wall_shape, dynamic=False, bounciness=2)
arena_bottom_wall = Rectangle2DNode(width=camera.viewport.width, height=area_wall_thickness, color=engine_draw.red)
arena_bottom_wall_physics.add_child(arena_bottom_wall)


arena_left_wall_physics = Physics2DNode(position=Vector2(-camera.viewport.width/2, 0), collision_shape=arena_vertical_wall_shape, dynamic=False, bounciness=2)
arena_left_wall = Rectangle2DNode(width=area_wall_thickness, height=camera.viewport.height, color=engine_draw.red)
arena_left_wall_physics.add_child(arena_left_wall)

arena_right_wall_physics = Physics2DNode(position=Vector2(camera.viewport.width/2, 0), collision_shape=arena_vertical_wall_shape, dynamic=False, bounciness=2)
arena_right_wall = Rectangle2DNode(width=area_wall_thickness, height=camera.viewport.height, color=engine_draw.red)
arena_right_wall_physics.add_child(arena_right_wall)


class PlayerPaddle(Physics2DNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector2(-54, 0)
        self.bounciness = 2

        self.paddle = Rectangle2DNode(width=5, height=25, color=engine_draw.green)
        self.add_child(self.paddle)

        self.collision_shape = PolygonCollisionShape2D()
        self.collision_shape.vertices.append(Vector2(-self.paddle.width/2, -self.paddle.height/2-1))
        self.collision_shape.vertices.append(Vector2( self.paddle.width/2, -self.paddle.height/2-1))
        self.collision_shape.vertices.append(Vector2( self.paddle.width/2,  self.paddle.height/2))
        self.collision_shape.vertices.append(Vector2(-self.paddle.width/2,  self.paddle.height/2))
        self.collision_shape.calculate_normals()
    
    def tick(self):
        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.velocity.y = -1.5
        elif engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.velocity.y = 1.5
        else:
            self.velocity.y = 0


player_paddle = PlayerPaddle()

# ball = Polygon2DNode(vertices=engine_utilities.generate_polygon_hexagon(3))
ball_physics = Physics2DNode(collision_shape=PolygonCollisionShape2D(vertices=engine_utilities.generate_polygon_hexagon(3)), bounciness=2)
ball = Circle2DNode(radius=3)
ball_physics.add_child(ball)
ball_physics.velocity.x = 1.5
ball_physics.velocity.y = -1

engine.start()