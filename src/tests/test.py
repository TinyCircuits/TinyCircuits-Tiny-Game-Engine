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


poly0 = Polygon2DNode()
poly0.vertices.append(Vector2(-7, -7))
poly0.vertices.append(Vector2(10, -5))
poly0.vertices.append(Vector2(5, 5))
poly0.vertices.append(Vector2(-15, 9))
poly0.color = 0b1111100000011111

poly1 = Polygon2DNode()
poly1.vertices.append(Vector2(-8, -11))
poly1.vertices.append(Vector2(5, -5))
poly1.vertices.append(Vector2(7, 9))
poly1.vertices.append(Vector2(-6, 5))
poly1.color = 0b1111100000000000


physics_poly0 = Physics2DNode()
physics_poly0.position = Vector2(64-32, 64)
physics_poly0.collision_shape = PolygonCollisionShape2D()
physics_poly0.collision_shape.vertices = poly0.vertices
physics_poly0.collision_shape.calculate_normals()
# physics_poly0.rotation = math.pi/4
physics_poly0.velocity = Vector2(0.75, 0)
physics_poly0.add_child(poly0)

physics_poly1 = Physics2DNode()
physics_poly1.position = Vector2(64+32, 64)
physics_poly1.collision_shape = PolygonCollisionShape2D()
physics_poly1.collision_shape.vertices = poly1.vertices
physics_poly1.collision_shape.calculate_normals()
# physics_poly1.rotation = math.pi/4
physics_poly1.velocity = Vector2(-0.75, 0)
physics_poly1.add_child(poly1)


camera = CameraNode()

engine.start()