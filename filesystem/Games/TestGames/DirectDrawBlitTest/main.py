import engine_main
import engine
import engine_draw
from engine_nodes import CameraNode
from engine_math import Vector2
from engine_resources import TextureResource

texture = TextureResource("Games/TestGames/DirectDrawBlitTest/32x32.bmp")

while True:
    engine_draw.clear(0)
    engine_draw.blit(texture, 32, 0,  0.75)
    engine_draw.blit(texture, 32, 32, 0.25)
    engine_draw.blit(texture, 32, 96, 1.0)
    engine_draw.update()