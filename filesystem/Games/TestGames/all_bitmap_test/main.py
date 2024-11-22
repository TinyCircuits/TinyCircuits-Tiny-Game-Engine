import engine_main

import engine
from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, CameraNode
from engine_math import Vector2
import engine_io

import os
cam = CameraNode()

os.chdir("Games/TestGames/all_bitmap_test")

# Textures
tex_1bit_2color = TextureResource("1bit_2color.bmp")
tex_4bit_16color = TextureResource("4bit_16color.bmp")
tex_8bit_256color = TextureResource("8bit_256color.bmp")
tex_4bit_redbrick_no_clr_used = TextureResource("REDBRICK.BMP")

tex_16bit_rgb_565 = TextureResource("16bit_rgb_565.bmp")
tex_16bit_xrgb_1555 = TextureResource("16bit_xrgb_1555.bmp")
tex_16bit_argb_1555 = TextureResource("16bit_argb_1555.bmp")
tex_16bit_xrgb_4444 = TextureResource("16bit_xrgb_4444.bmp")
tex_16bit_argb_4444 = TextureResource("16bit_argb_4444.bmp")
tex_16bit_rgb_565_no_bf_image_size = TextureResource("16bit_rgb_565_no_bf_image_size.bmp")

tex_checkpoint = TextureResource("checkpoint.bmp")
tex_player_move_left = TextureResource("player_move_left.bmp")


# Sprites

# Top row
spr_1bit_2color = Sprite2DNode(texture=tex_1bit_2color, position=Vector2(0, 0))
spr_4bit_16color = Sprite2DNode(texture=tex_4bit_16color, position=Vector2(18, 0))
spr_8bit_256color = Sprite2DNode(texture=tex_8bit_256color, position=Vector2(36, 0))
spr_4bit_redbrick_no_clr_used = Sprite2DNode(texture=tex_4bit_redbrick_no_clr_used, position=Vector2(64, -10))

# Bottom row
spr_16bit_rgb_565 = Sprite2DNode(texture=tex_16bit_rgb_565, position=Vector2(0, 18))
spr_16bit_xrgb_1555 = Sprite2DNode(texture=tex_16bit_xrgb_1555, position=Vector2(18, 18))
spr_16bit_argb_1555 = Sprite2DNode(texture=tex_16bit_argb_1555, position=Vector2(36, 18))

spr_16bit_xrgb_4444 = Sprite2DNode(texture=tex_16bit_xrgb_4444, position=Vector2(54, 18))
spr_16bit_xrgb_4444.scale.x = 16/tex_16bit_xrgb_4444.width
spr_16bit_xrgb_4444.scale.y = 16/tex_16bit_xrgb_4444.height

spr_16bit_argb_4444 = Sprite2DNode(texture=tex_16bit_argb_4444, position=Vector2(72, 18))
spr_16bit_argb_4444.scale.x = 16/tex_16bit_argb_4444.width
spr_16bit_argb_4444.scale.y = 16/tex_16bit_argb_4444.height

spr_16bit_rgb_565_no_bf_image_size = Sprite2DNode(texture=tex_16bit_rgb_565_no_bf_image_size, position=Vector2(105, 24))

spr_checkpoint = Sprite2DNode(texture=tex_checkpoint, position=Vector2(-70, -14))
spr_player_move_left = Sprite2DNode(texture=tex_player_move_left, position=Vector2(-70, 14))


# Odd number dimensions sprites
bg_bmp1 = TextureResource("scoreBG1.bmp", False)
bg_spr1 = Sprite2DNode(position=Vector2(-30, -50), texture=bg_bmp1, opacity=1)

bg_bmp4 = TextureResource("scoreBG4.bmp", False)
bg_spr4 = Sprite2DNode(position=Vector2(-30, -20), texture=bg_bmp4, opacity=1)

bg_bmp8 = TextureResource("scoreBG8.bmp", False)
bg_spr8 = Sprite2DNode(position=Vector2(-30, 10), texture=bg_bmp8, opacity=1)

bg_bmp16 = TextureResource("scoreBG16.bmp", False)
bg_spr16 = Sprite2DNode(position=Vector2(-30, 40), texture=bg_bmp16, opacity=1)


while True:
    if engine.tick() is False:
        continue

    # print(engine.get_running_fps())

    if engine_io.LEFT.is_pressed:
        cam.position.x -= 0.5
    elif engine_io.RIGHT.is_pressed:
        cam.position.x += 0.5
    
    if engine_io.DOWN.is_pressed:
        cam.position.y += 0.5
    elif engine_io.UP.is_pressed:
        cam.position.y -= 0.5
    
    if engine_io.LB.is_pressed:
        cam.zoom -= 0.075
    elif engine_io.RB.is_pressed:
        cam.zoom += 0.075