import engine
import engine_draw
from engine_math import Vector2
from engine_physics import PolyCollisionShape2D
from engine_nodes import Polygon2DNode, Rectangle2DNode, Circle2DNode, CameraNode, Physics2DNode

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

arena_horizontal_wall_shape = PolyCollisionShape2D(vertices=arena_horizontal_poly_verts)
arena_vertical_wall_shape = PolyCollisionShape2D(vertices=arena_vertical_poly_verts)


arena_top_wall_physics = Physics2DNode(position=Vector2(0, -camera.viewport.height/2), collision_shape=arena_horizontal_wall_shape, dynamic=False)
arena_top_wall = Rectangle2DNode(width=camera.viewport.width, height=area_wall_thickness, color=engine_draw.red)
arena_top_wall_physics.add_child(arena_top_wall)

arena_bottom_wall_physics = Physics2DNode(position=Vector2(0, camera.viewport.height/2), collision_shape=arena_horizontal_wall_shape, dynamic=False)
arena_bottom_wall = Rectangle2DNode(width=camera.viewport.width, height=area_wall_thickness, color=engine_draw.red)
arena_bottom_wall_physics.add_child(arena_bottom_wall)


arena_left_wall_physics = Physics2DNode(position=Vector2(-camera.viewport.width/2, 0), collision_shape=arena_vertical_wall_shape, dynamic=False)
arena_left_wall = Rectangle2DNode(width=area_wall_thickness, height=camera.viewport.height, color=engine_draw.red)
arena_left_wall_physics.add_child(arena_left_wall)

arena_right_wall_physics = Physics2DNode(position=Vector2(camera.viewport.width/2, 0), collision_shape=arena_vertical_wall_shape, dynamic=False)
arena_right_wall = Rectangle2DNode(width=area_wall_thickness, height=camera.viewport.height, color=engine_draw.red)
arena_right_wall_physics.add_child(arena_right_wall)


engine.start()