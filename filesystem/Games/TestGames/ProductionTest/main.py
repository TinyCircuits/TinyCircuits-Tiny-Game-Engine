import engine_main
import engine
import engine_io
import engine_audio
from engine_resources import ToneSoundResource, FontResource
from engine_nodes import CameraNode, Text2DNode, Circle2DNode

cam = CameraNode()
text = Text2DNode(text="TEST")

class Button():
    def __init__(self, button):
        self.btn = button
        self.pressed_before = False


buttons = [
    Button(engine_io.A),
    Button(engine_io.B),
    Button(engine_io.UP),
    Button(engine_io.DOWN),
    Button(engine_io.LEFT),
    Button(engine_io.RIGHT),
    Button(engine_io.LB),
    Button(engine_io.RB),
    Button(engine_io.MENU),
]


while True:
    if engine.tick():
        for button in buttons:
            if button.btn.is_pressed:
                button.pressed_before = True