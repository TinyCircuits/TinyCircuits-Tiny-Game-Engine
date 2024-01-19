import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_physics import CircleCollisionShape2D, RectangleCollisionShape2D, PolygonCollisionShape2D
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Polygon2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource
import math

engine.set_fps_limit(60)
engine_physics.set_physics_fps_limit(60)


poly = Polygon2DNode()
poly.vertices.append(Vector2(-5, -5))
poly.vertices.append(Vector2(5, -5))
poly.vertices.append(Vector2(5, 5))
poly.vertices.append(Vector2(-5, 5))

physics_poly = Physics2DNode()
physics_poly.position = Vector2(64, 64)
physics_poly.collision_shape = PolygonCollisionShape2D()
physics_poly.collision_shape.vertices = poly.vertices
physics_poly.rotation = math.pi/4
physics_poly.add_child(poly)

print(poly.vertices)


camera = CameraNode()
engine.start()