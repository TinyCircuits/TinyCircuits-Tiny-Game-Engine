import engine
import engine_draw
import engine_debug
import engine_input
import engine_physics
from engine_nodes import EmptyNode, Sprite2DNode, Rectangle2DNode, Circle2DNode, CameraNode, VoxelSpaceNode, Physics2DNode
from engine_math import Vector3, Vector2, Rectangle
from engine_resources import TextureResource


# engine.set_fps_limit(60)
engine_physics.set_physics_fps_limit(60)

pool_table_texture = TextureResource("pool_table.bmp")
sprite = Sprite2DNode(pool_table_texture)
sprite.transparent_color = engine_draw.white


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        if engine_input.is_a_pressed():
            self.zoom -= 0.01
            print(self.zoom)
        if engine_input.is_b_pressed():
            self.zoom += 0.01
            print(self.zoom)
        
        if engine_input.is_dpad_up_pressed():
            self.position.y -= 1
        if engine_input.is_dpad_down_pressed():
            self.position.y += 1
        
        if engine_input.is_dpad_left_pressed():
            self.position.x -= 1
        if engine_input.is_dpad_right_pressed():
            self.position.x += 1
        
        
        if engine_input.is_bumper_left_pressed():
            self.rotation.z += 0.1
        if engine_input.is_bumper_right_pressed():
            self.rotation.z -= 0.1


camera = MyCam()
engine.start()