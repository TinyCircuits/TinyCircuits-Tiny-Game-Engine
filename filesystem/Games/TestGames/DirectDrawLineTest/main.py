import engine_main
import engine
import engine_draw
from engine_nodes import CameraNode
from engine_math import Vector2


while True:
    engine_draw.clear(0)
    engine_draw.line(engine_draw.white, 32, 0,   32, 32,   0.75)
    engine_draw.line(engine_draw.white, 32, 32,  32, 96,   0.5)
    engine_draw.line(engine_draw.white, 32, 96,  32, 128,  1.0)
    engine_draw.update()