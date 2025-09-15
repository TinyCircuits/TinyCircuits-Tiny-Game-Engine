import engine
from engine_nodes import CameraNode, Text2DNode
from engine_resources import FontResource

engine.fps_limit(60)

font = FontResource("/system/assets/outrunner_outline.bmp")

error_text = Text2DNode(font=font, text="ERROR: Something\nwent wrong\nwhile executing\nprogram...", line_spacing=1)

camera = CameraNode()

ticks = 0
while ticks < 60 * 2:
    if engine.tick():
        ticks += 1
