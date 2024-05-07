import engine_main

import engine
import engine_draw
from engine_draw import Color
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource
import math


class AnimatedJumpingSprite(Sprite2DNode):
    def __init__(self, texture):
        super().__init__(self)
        self.texture = texture
        self.time = 0
        self.scale.x = 0.6 
        self.scale.y = 0.6


background = Color(0.647, 0.612, 0.808)
engine_draw.set_background_color(background)

gear_texture = TextureResource("gear.bmp")
gear_sprite = Sprite2DNode(texture=gear_texture)
gear_sprite.position.x = 32
gear_sprite.position.y = 32

dpad_direction_texture = TextureResource("dpad_direction.bmp")
dpad_direction = AnimatedJumpingSprite(dpad_direction_texture)

texture = TextureResource("thumby_color.bmp")
sprite = Sprite2DNode(texture=texture, rotation=math.pi/6)
sprite.position.x = 64
sprite.position.y = 42

camera = CameraNode()

# engine.start()
while True:
    # print(engine.get_running_fps())
    engine.tick()