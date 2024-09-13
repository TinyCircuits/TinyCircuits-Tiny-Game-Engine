import engine_main
import engine
import engine_io
import mvf
from engine_resources import FontResource
from engine_nodes import Text2DNode, CameraNode
from engine_math import Vector2

camera = CameraNode()

def callback():
    if engine_io.A.is_just_pressed: mvf.printmem()
    if engine_io.B.is_just_pressed:
        mvf.stop()
    if engine_io.RB.is_just_pressed:
        fps.toggle()
    if engine_io.LB.is_just_pressed:
        mvf.capframerate = False

font = FontResource("Portfolio6x8.bmp")
class FPStext(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.font = font
        self.visible = False
        self.opacity = 0
        self.position = Vector2(0,-59)
        self.frame = 0
        self.total = 0
        self.windowsize = 5
    
    def tick(self, dt):
        if self.opacity > 0:
            if self.frame == self.windowsize:
                self.text = f"{self.total / self.windowsize} FPS"
                self.frame = 0
                self.total = 0
            self.frame += 1
            self.total += engine.get_running_fps()
    
    def toggle(self):
        if self.visible:
            self.opacity = 0
        else:
            self.opacity = 1
        self.visible = not self.visible

fps = FPStext()

vf = open("badapple-128x96.mvf", "rb")
mvf.load(vf)
mvf.play(callback=callback)


print("Exiting...")