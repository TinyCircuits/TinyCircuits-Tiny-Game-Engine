import engine
from engine_nodes import GUIButton2DNode, CameraNode
from engine_resources import FontResource
from engine_math import Vector2
import engine_draw
import engine_input
from engine_animation import Tween, ONE_SHOT, LOOP, PING_PONG, EASE_CIRC_IN_OUT, EASE_BOUNCE_IN, EASE_BOUNCE_OUT

engine.set_fps_limit(120)

font = FontResource("9pt-roboto-font.bmp")

# engine_input.gui_toggle_button = None

# button0 = GUIButton2DNode(position=Vector2(-32,   0), font=font, text="Button 0", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button1 = GUIButton2DNode(position=Vector2(  0, -32), font=font, text="Button 1", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button2 = GUIButton2DNode(position=Vector2( 32,   0), font=font, text="Button 2", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button3 = GUIButton2DNode(position=Vector2(  0,  32), font=font, text="Button 3", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)



class MyButton(GUIButton2DNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        pass
    
    def on_focused(self):
        print("Focused", self.text)
    
    def on_just_focused(self):
        print("Just focused", self.text)
    
    def on_just_unfocused(self):
        print("Just unfocused", self.text)

    def on_pressed(self):
        print("Pressed!", self.text)
    
    def on_just_pressed(self):
        print("Just pressed!", self.text)
    
    def on_just_released(self):
        print("Just released!", self.text)
    
    def on_just_changed(self):
        print("Just changed!", self.text)



buttons = []
for x in range(5):
    for y in range(5):
        button = MyButton()
        button.position = Vector2(x*30, y*30)
        button.font = font
        button.text = str(x) + str(y)
        button.rotation = 0
        button.scale = Vector2(1, 1)
        button.padding = 4
        button.outline = 4
        button.opacity = 1.0
        buttons.append(button)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.zoom = 1

    
    def tick(self):
        if(engine_input.focused != None):
            self.position.x = engine_input.focused.position.x
            self.position.y = engine_input.focused.position.y

        if engine_input.check_pressed(engine_input.B):
            self.zoom -= 0.025
        if engine_input.check_pressed(engine_input.A):
            self.zoom += 0.025
        
        if engine_input.check_pressed(engine_input.DPAD_UP):
            self.position.y -= 0.5
        if engine_input.check_pressed(engine_input.DPAD_DOWN):
            self.position.y += 0.5
        
        if engine_input.check_pressed(engine_input.DPAD_LEFT):
            self.position.x -= 0.5
        if engine_input.check_pressed(engine_input.DPAD_RIGHT):
            self.position.x += 0.5
        
        if engine_input.check_pressed(engine_input.BUMPER_LEFT):
            self.rotation.z += 0.0085
        if engine_input.check_pressed(engine_input.BUMPER_RIGHT):
            self.rotation.z -= 0.0085


camera = MyCam()

# camera.add_child(button0)
# camera.add_child(button1)

engine.start()