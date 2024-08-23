import engine_main
import engine
from engine_nodes import CameraNode, Sprite2DNode
from engine_resources import TextureResource
import engine_draw
from engine_math import Vector2

camera = CameraNode()

blank_texture_default = TextureResource(16, 16)
blank_texture_const_color = TextureResource(16, 16, engine_draw.green)
blank_texture_custom_color = TextureResource(16, 16, engine_draw.Color(0.5, 0.5, 0.5))

spr_default = Sprite2DNode(texture=blank_texture_default, position=Vector2(-32, 0))
spr_const_color = Sprite2DNode(texture=blank_texture_const_color, position=Vector2(0, 0))
spr_custom_color = Sprite2DNode(texture=blank_texture_custom_color, position=Vector2(32, 0))

engine.start()