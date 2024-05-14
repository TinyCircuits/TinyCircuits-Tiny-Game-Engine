import engine
import engine_io
from engine_nodes import Rectangle2DNode, Text2DNode, CameraNode
from engine_resources import FontResource
from engine_math import Vector2

# Create a rectangle that is not child of anything
rectangle = Rectangle2DNode()

# Create a `Text2DNode` that will be a child of the camera
# Once the text is a child of the camera it's position will
# be in the local space of the camera.
font = FontResource("mono-font-10pt.bmp")
hud_text = Text2DNode(font=font, text="HUD!", position=Vector2(-34, -54))

# Create a camera class that allows for basic traversal
class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        if engine_io.check_pressed(engine_io.A):
            self.zoom -= 0.0025
        if engine_io.check_pressed(engine_io.B):
            self.zoom += 0.0025
        
        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.position.y -= 0.25
        if engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.position.y += 0.25
        
        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.position.x -= 0.25
        if engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.position.x += 0.25
        
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            self.rotation.z += 0.005
        if engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            self.rotation.z -= 0.005

# Make the camera and add the text as a child of it
camera = MyCam()
camera.add_child(hud_text)

engine.start()
