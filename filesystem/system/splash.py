import engine
import engine_draw
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource
from engine_animation import Tween, Delay, ONE_SHOT, EASE_ELAST_OUT
from engine_math import Vector2
import random

engine.set_fps_limit(60)

# Camera to render scene
camera = CameraNode()

class MainDelay(Delay):
    def __init__(self):
        super().__init__(self)

        self.T_texture = TextureResource("assets/T.bmp")
        self.T_sprite = Sprite2DNode(texture=self.T_texture, position=Vector2(-53, -100), transparent_color = engine_draw.black)
        self.T_tween = Tween()

        self.h_texture = TextureResource("assets/h.bmp")
        self.h_sprite = Sprite2DNode(texture=self.h_texture, position=Vector2(-44, -100), transparent_color = engine_draw.black)
        self.h_tween = Tween()

        self.u_texture = TextureResource("assets/u.bmp")
        self.u_sprite = Sprite2DNode(texture=self.u_texture, position=Vector2(-34, -100), transparent_color = engine_draw.black)
        self.u_tween = Tween()

        self.m_texture = TextureResource("assets/m.bmp")
        self.m_sprite = Sprite2DNode(texture=self.m_texture, position=Vector2(-24, -100), transparent_color = engine_draw.black)
        self.m_tween = Tween()

        self.b_texture = TextureResource("assets/b.bmp")
        self.b_sprite = Sprite2DNode(texture=self.b_texture, position=Vector2(-13, -100), transparent_color = engine_draw.black)
        self.b_tween = Tween()

        self.y_texture = TextureResource("assets/y.bmp")
        self.y_sprite = Sprite2DNode(texture=self.y_texture, position=Vector2(-4, -100), transparent_color = engine_draw.black)
        self.y_tween = Tween()

        self.C_texture = TextureResource("assets/C.bmp")
        self.C_sprite = Sprite2DNode(texture=self.C_texture, position=Vector2(10, -100), transparent_color = engine_draw.black)
        self.C_tween = Tween()

        self.GO_texture = TextureResource("assets/GO.bmp")
        self.GO_sprite = Sprite2DNode(texture=self.GO_texture, position=Vector2(21, -100), transparent_color = engine_draw.black)
        self.GO_tween = Tween()

        self.L_texture = TextureResource("assets/L.bmp")
        self.L_sprite = Sprite2DNode(texture=self.L_texture, position=Vector2(31, -100), transparent_color = engine_draw.black)
        self.L_tween = Tween()

        self.RO_texture = TextureResource("assets/RO.bmp")
        self.RO_sprite = Sprite2DNode(texture=self.RO_texture, position=Vector2(41, -100), transparent_color = engine_draw.black)
        self.RO_tween = Tween()

        self.R_texture = TextureResource("assets/R.bmp")
        self.R_sprite = Sprite2DNode(texture=self.R_texture, position=Vector2(52, -100), transparent_color = engine_draw.black)
        self.R_tween = Tween()


    def after(self):
        self.T_tween.start(self.T_sprite, "position", None, Vector2(-53, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.h_tween.start(self.h_sprite, "position", None, Vector2(-44, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.u_tween.start(self.u_sprite, "position", None, Vector2(-34, 2), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.m_tween.start(self.m_sprite, "position", None, Vector2(-24, 2), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.b_tween.start(self.b_sprite, "position", None, Vector2(-13, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.y_tween.start(self.y_sprite, "position", None, Vector2(-4, 4), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.C_tween.start(self.C_sprite, "position", None, Vector2(10, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.GO_tween.start(self.GO_sprite, "position", None, Vector2(21, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.L_tween.start(self.L_sprite, "position", None, Vector2(31, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.RO_tween.start(self.RO_sprite, "position", None, Vector2(41, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)
        self.R_tween.start(self.R_sprite, "position", None, Vector2(52, -1), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

# Need to give the screen time to setup, wait 650ms before starting animation
delay = MainDelay()
delay.start(650.0)

while delay.finished == False or delay.T_tween.finished == False:
    engine.tick()

# Need to stop all tweens since reference sprites will be gc collected
delay.T_tween.pause()
delay.h_tween.pause()
delay.u_tween.pause()
delay.m_tween.pause()
delay.b_tween.pause()
delay.y_tween.pause()
delay.C_tween.pause()
delay.GO_tween.pause()
delay.L_tween.pause()
delay.RO_tween.pause()
delay.R_tween.pause()
