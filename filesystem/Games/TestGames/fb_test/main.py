import engine_main

import engine
import engine_draw

import framebuf

engine.fps_limit(60)

while True:
    if engine.tick():
        engine_draw.front_fb.fill(0xffff)
        engine_draw.front_fb.pixel(64, 64, 0b0000011111100000)
        # engine_draw.back_fb.fill(0xffff)