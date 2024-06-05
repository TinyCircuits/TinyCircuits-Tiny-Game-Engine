import engine_main
import engine

from engine_resources import FontResource
from engine_nodes import Text2DNode, CameraNode
import engine_draw

engine.disable_fps_limit()

roboto_font = FontResource("9pt-roboto-font.bmp")

text = Text2DNode(text="Hello!", font=roboto_font, color=engine_draw.red)

cam = CameraNode()

while True:
    if engine.tick():
        print(engine.get_running_fps())