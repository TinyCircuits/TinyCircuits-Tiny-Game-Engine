import engine_main

import engine
from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, CameraNode

import os
os.chdir("Games/all_bitmap_test")

tex_1bit_2color = TextureResource("1bit_2color.bmp")
tex_4bit_16color = TextureResource("4bit_16color.bmp")
tex_8bit_256color = TextureResource("8bit_256color.bmp")

tex_16bit_rgb_565 = TextureResource("16bit_rgb_565.bmp")
tex_16bit_xrgb_1555 = TextureResource("16bit_xrgb_1555.bmp")
tex_16bit_argb_1555 = TextureResource("16bit_argb_1555.bmp")

tex_24bit_rgb_888 = TextureResource("24bit_rgb_888.bmp")

tex_32bit_xrgb_8888 = TextureResource("32bit_xrgb_8888.bmp")
tex_32bit_argb_8888 = TextureResource("32bit_argb_8888.bmp")

tex_16bit_rgb_565_no_bf_image_size = TextureResource("16bit_rgb_565_no_bf_image_size.bmp")
tex_4bit_redbrick_no_clr_used = TextureResource("REDBRICK.BMP")