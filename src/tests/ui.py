import engine
from engine_nodes import Button2DNode, CameraNode
from engine_resources import FontResource

font = FontResource("9pt-roboto-font.bmp")
button = Button2DNode()
camera = CameraNode()

engine.start()