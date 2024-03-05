import engine
import engine_draw
import engine_input
import engine_physics
from engine_physics import PolygonCollisionShape2D
from engine_math import Vector2
from engine_nodes import Polygon2DNode, Rectangle2DNode, Circle2DNode, CameraNode, Physics2DNode

# Imagine we're targeting a top-down game, turn gravity off on each axis
engine_physics.set_gravity(0, 0)

# Create lists of Vector2s representing horizontal and vertical rectangles
# the same size as the screen
wall_thickness = 5

horizontal_poly_verts = [   Vector2(-128/2,   -wall_thickness),
                            Vector2( 128/2-1, -wall_thickness),
                            Vector2( 128/2-1,  wall_thickness),
                            Vector2(-128/2,    wall_thickness)]

vertical_poly_verts =   [   Vector2(-wall_thickness,   -128/2),
                            Vector2( wall_thickness,   -128/2),
                            Vector2( wall_thickness,    128/2-1),
                            Vector2(-wall_thickness,    128/2-1)]

# Need `PolygonCollisionShape2D` shapes that will be used with our `Physics2DNodes`
horizontal_wall_shape = PolygonCollisionShape2D(vertices=horizontal_poly_verts)
vertical_wall_shape = PolygonCollisionShape2D(vertices=vertical_poly_verts)

# Make `Physics2DNodes` and position them at edges of the screen (assuming camera is at 0,0). Set dynamic False
# so that the walls are no moved by gravity or collision
top_wall_physics_node = Physics2DNode(position=Vector2(0, -128/2), collision_shape=horizontal_wall_shape, dynamic=False)
bottom_wall_physics_node = Physics2DNode(position=Vector2(0, 128/2), collision_shape=horizontal_wall_shape, dynamic=False)
left_wall_physics_node = Physics2DNode(position=Vector2(-128/2, 0), collision_shape=horizontal_wall_shape, dynamic=False)
right_wall_physics_node = Physics2DNode(position=Vector2(128/2, 0), collision_shape=horizontal_wall_shape, dynamic=False)

# Make some `Rectangle2DNode` and add them as children to the physics nodes and make them
# the same size as the colliders to represent the extents
top_wall = Rectangle2DNode(width=128, height=wall_thickness, color=engine_draw.red)
bottom_wall = Rectangle2DNode(width=128, height=wall_thickness, color=engine_draw.red)
left_wall = Rectangle2DNode(width=wall_thickness, height=128, color=engine_draw.red)
right_wall = Rectangle2DNode(width=wall_thickness, height=128, color=engine_draw.red)

top_wall_physics_node.add_child(top_wall)
bottom_wall_physics_node.add_child(bottom_wall)
left_wall_physics_node.add_child(left_wall)
right_wall_physics_node.add_child(right_wall)


# 
class Player(Physics2DNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector2(0, 0)

        self.player = Rectangle2DNode(width=5, height=10, color=engine_draw.green)
        self.add_child(self.player)

        self.collision_shape = PolygonCollisionShape2D()
        self.collision_shape.vertices.append(Vector2(-self.player.width/2, -self.player.height/2-1))
        self.collision_shape.vertices.append(Vector2( self.player.width/2, -self.player.height/2-1))
        self.collision_shape.vertices.append(Vector2( self.player.width/2,  self.player.height/2))
        self.collision_shape.vertices.append(Vector2(-self.player.width/2,  self.player.height/2))
        self.collision_shape.calculate_normals()
    
    def tick(self):
        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.velocity.y = -1.5
        elif engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.velocity.y = 1.5
        else:
            self.velocity.y = 0


player = Player()

camera = CameraNode()

engine.start()
