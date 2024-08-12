import engine_main
import engine

from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, CameraNode

engine.disable_fps_limit()

texture = TextureResource("i.bmp")

spr = Sprite2DNode(texture=texture)

cam = CameraNode()

while True:
    if engine.tick():
        print(engine.get_running_fps())