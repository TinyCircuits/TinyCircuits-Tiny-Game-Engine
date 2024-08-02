from engine_nodes import CameraNode
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT


class CustomCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.camera_tween_x = Tween()
        self.camera_tween_y = Tween()
        self.index = 0
    
    def switch(self, dir):
        if self.camera_tween_x.finished == False:
            return

        self.index += dir

        if(self.index >= 3):
            self.index = 0
        elif(self.index < 0):
            self.index = 2

        self.goto_x(self.index*128.0)

    def goto_x(self, x):
        self.camera_tween_x.start(self.position, "x", self.position.x, x, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)
    
    def goto_y(self, y):
        self.camera_tween_y.start(self.position, "y", self.position.y, y, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)