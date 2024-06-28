import engine_main

import engine
from engine_nodes import GUIButton2DNode, GUIBitmapButton2DNode, CameraNode
from engine_resources import FontResource, TextureResource
from engine_math import Vector2
import engine_draw
import engine_io
from engine_animation import Tween, ONE_SHOT, LOOP, PING_PONG, EASE_CIRC_IN_OUT, EASE_BOUNCE_IN, EASE_BOUNCE_OUT

engine.fps_limit(120)

font = FontResource("9pt-roboto-font.bmp")

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.zoom = 1


    def tick(self, dt):
        if(engine_io.focused_node() != None):
            self.position.x = engine_io.focused_node().position.x
            self.position.y = engine_io.focused_node().position.y


        if engine_io.B.is_pressed:
            self.zoom -= 0.025
        if engine_io.A.is_pressed:
            self.zoom += 0.025

        if engine_io.UP.is_pressed:
            self.position.y -= 0.5
        if engine_io.DOWN.is_pressed:
            self.position.y += 0.5

        if engine_io.LEFT.is_pressed:
            self.position.x -= 0.5
        if engine_io.RIGHT.is_pressed:
            self.position.x += 0.5

        if engine_io.LB.is_pressed:
            self.rotation.z += 0.0085
        if engine_io.RB.is_pressed:
            self.rotation.z -= 0.0085


camera = MyCam()

# engine_io.gui_toggle_button(None)

# button0 = GUIButton2DNode(position=Vector2(-32,   0), font=font, text="Button 0", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button1 = GUIButton2DNode(position=Vector2(  0, -32), font=font, text="Button 1", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button2 = GUIButton2DNode(position=Vector2( 32,   0), font=font, text="Button 2", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)
# button3 = GUIButton2DNode(position=Vector2(  0,  32), font=font, text="Button 3", rotation=0, scale=Vector2(1, 1), padding=2, outline=2, opacity=1.0)

bitmap = TextureResource("button.bmp")
bitmap_focused = TextureResource("button-highlighted.bmp")
bitmap_pressed = TextureResource("button-pressed.bmp")
bitmap_btn = GUIBitmapButton2DNode(position=Vector2(0, -32), font=font, text="Test", text_scale=Vector2(1.0, 1.0), bitmap=bitmap, focused_bitmap=bitmap_focused, pressed_bitmap=bitmap_pressed)


class MyButton(GUIButton2DNode):
    def __init__(self):
        super().__init__(self)

    def tick(self, dt):
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


# camera.add_child(button0)
# camera.add_child(button1)

engine.start()