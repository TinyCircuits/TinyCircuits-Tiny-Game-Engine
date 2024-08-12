from engine_nodes import CameraNode
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT


class CustomCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.tween = Tween()
    
    def to_page(self, page_index):
        if not self.tween.finished:
            return False

        self.goto_x(page_index*128.0)
        return True

    def goto_x(self, x):
        self.tween.start(self.position, "x", self.position.x, x, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)