import engine_io
from engine_nodes import Text2DNode


class DirectionIcon(Text2DNode):
    def __init__(self, text, font, button, position):
        super().__init__(self)
        self.text = text
        self.font = font
        self.button = button
        self.position = position
        self.layer = 3
    
    def tick(self, dt):
        if self.button.is_pressed:
            self.opacity = 0.65
        else:
            self.opacity = 1.0