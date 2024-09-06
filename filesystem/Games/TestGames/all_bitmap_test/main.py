import engine_main

import engine
from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, CameraNode
from engine_math import Vector2
import engine_io

import os
os.chdir("Games/TestGames/all_bitmap_test")
cam = CameraNode()

# Textures
tex_1bit_2color = TextureResource("1bit_2color.bmp")
tex_4bit_16color = TextureResource("4bit_16color.bmp")
tex_8bit_256color = TextureResource("8bit_256color.bmp")

tex_16bit_rgb_565 = TextureResource("16bit_rgb_565.bmp")
tex_16bit_xrgb_1555 = TextureResource("16bit_xrgb_1555.bmp")
tex_16bit_argb_1555 = TextureResource("16bit_argb_1555.bmp")
tex_16bit_xrgb_4444 = TextureResource("16bit_xrgb_4444.bmp")
tex_16bit_argb_4444 = TextureResource("16bit_argb_4444.bmp")

tex_24bit_rgb_888 = TextureResource("24bit_rgb_888.bmp")

tex_32bit_xrgb_8888 = TextureResource("32bit_xrgb_8888.bmp")
tex_32bit_argb_8888 = TextureResource("32bit_argb_8888.bmp")

tex_16bit_rgb_565_no_bf_image_size = TextureResource("16bit_rgb_565_no_bf_image_size.bmp")
tex_4bit_redbrick_no_clr_used = TextureResource("REDBRICK.BMP")

# Sprites
spr_1bit_2color = Sprite2DNode(texture=tex_1bit_2color, position=Vector2(0, 0))
spr_4bit_16color = Sprite2DNode(texture=tex_4bit_16color, position=Vector2(18, 0))
spr_8bit_256color = Sprite2DNode(texture=tex_8bit_256color, position=Vector2(36, 0))

spr_16bit_rgb_565 = Sprite2DNode(texture=tex_16bit_rgb_565, position=Vector2(0, 18))

while True:
    if engine.tick() is False:
        continue

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