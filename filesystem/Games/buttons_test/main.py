import engine_main
from engine_nodes import CameraNode, Text2DNode
from engine_resources import FontResource
import engine
from engine_draw import Color
import engine_io
from system.root_dir import ROOT_DIR

engine.fps_limit(15)
engine_io.gui_toggle_button(None)

font = FontResource(f"{ROOT_DIR}/assets/outrunner_outline.bmp")

text = Text2DNode(
    text="Status of A is\nprinted to\nconsole.\n\nPress B to\ncall A.release()\n\nMENU - exit",
    font=font,
    line_spacing=1,
    color=Color(0.2, 0.7, 0.6),
)

cam = CameraNode()

count = 0

while True:
    if engine.tick():
        if engine_io.B.is_just_pressed:
            print("A.release()")
            engine_io.A.release()
        engine_io.A.print_info(not count)
        count = (count + 1) % 30
        if engine_io.MENU.is_just_short_released:
            break
