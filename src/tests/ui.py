import engine
from engine_nodes import Button2DNode, CameraNode
from engine_resources import FontResource
from engine_math import Vector2
import engine_input

font = FontResource("9pt-roboto-font.bmp")
# button = Button2DNode(font=font, text="Hi\nYo!", rotation=0)
button2 = Button2DNode(font=font, text="Hiiiiii\nYo!", rotation=0, position=Vector2(30, 0), scale=Vector2(1, 1))



class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        if engine_input.check_pressed(engine_input.A):
            self.zoom -= 0.025
        if engine_input.check_pressed(engine_input.B):
            self.zoom += 0.025
        
        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.position.y -= 0.25
        if engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.position.y += 0.25
        
        if engine_input.check_pressed(engine_input.DPAD_LEFT):
            self.position.x -= 0.25
        if engine_input.check_pressed(engine_input.DPAD_RIGHT):
            self.position.x += 0.25
        
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation.z += 0.005
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation.z -= 0.005
camera = MyCam()

engine.start()