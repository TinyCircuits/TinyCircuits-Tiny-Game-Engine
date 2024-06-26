import engine_main

import engine
import engine_io
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource

import framebuf

texture = TextureResource("32x32.bmp", True)
fb = framebuf.FrameBuffer(texture.data, texture.width, texture.height, framebuf.RGB565)

spr = Sprite2DNode(texture=texture)
cam = CameraNode()

count = 0

while True:
    if engine.tick():
        if engine_io.A.is_just_pressed:
            fb.fill(0xffff)

        count = count+1
        print(count)
