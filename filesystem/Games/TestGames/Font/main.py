import engine_main
import engine
from engine_resources import FontResource
from engine_nodes import Text2DNode, CameraNode
from engine_math import Vector2

roboto_font = FontResource("/Games/TestGames/Font/Portfolio6x8-1bit.bmp")
# roboto_font = FontResource("/Games/TestGames/Font/9pt-roboto-font.bmp")

class MessageSprite(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.text = "Hello"
        self.layer = 7
        self.position = Vector2(0, 0)
        self.font = roboto_font
        self.opacity = 1.0
        self.scale.x = 2
        self.scale.y = 2

current_msg = MessageSprite()

cam = CameraNode()

engine.start()