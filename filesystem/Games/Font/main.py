import engine_main

from engine_resources import FontResource
from engine_nodes import Text2DNode
from engine_math import Vector2

roboto_font = FontResource("9pt-roboto-font.bmp")

class MessageSprite(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.text = "testing"
        self.set_layer(7)
        self.position = Vector2(0, 0)
        self.font = roboto_font
        self.opacity = 1.0

current_msg = MessageSprite()