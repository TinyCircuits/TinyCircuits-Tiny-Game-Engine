import engine
import engine_draw
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT
from engine_math import Vector2
import random



class MainDelay(Delay):
    def __init__(self):
        super().__init__(self)
    
    def after(self):
        self.T_texture = TextureResource("T.bmp")
        self.T_sprite = Sprite2DNode(texture=self.T_texture, position=Vector2(-53, -100), transparent_color = engine_draw.black)
        self.T_tween = Tween()
        self.T_tween.start(self.T_sprite, "position", self.T_sprite.position, Vector2(-53, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.h_texture = TextureResource("h.bmp")
        self.h_sprite = Sprite2DNode(texture=self.h_texture, position=Vector2(-44, -100), transparent_color = engine_draw.black)
        self.h_tween = Tween()
        self.h_tween.start(self.h_sprite, "position", self.h_sprite.position, Vector2(-44, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.u_texture = TextureResource("u.bmp")
        self.u_sprite = Sprite2DNode(texture=self.u_texture, position=Vector2(-34, -100), transparent_color = engine_draw.black)
        self.u_tween = Tween()
        self.u_tween.start(self.u_sprite, "position", self.u_sprite.position, Vector2(-34, 2), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.m_texture = TextureResource("m.bmp")
        self.m_sprite = Sprite2DNode(texture=self.m_texture, position=Vector2(-24, -100), transparent_color = engine_draw.black)
        self.m_tween = Tween()
        self.m_tween.start(self.m_sprite, "position", self.m_sprite.position, Vector2(-24, 2), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.b_texture = TextureResource("b.bmp")
        self.b_sprite = Sprite2DNode(texture=self.b_texture, position=Vector2(-13, -100), transparent_color = engine_draw.black)
        self.b_tween = Tween()
        self.b_tween.start(self.b_sprite, "position", self.b_sprite.position, Vector2(-13, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.y_texture = TextureResource("y.bmp")
        self.y_sprite = Sprite2DNode(texture=self.y_texture, position=Vector2(-4, -100), transparent_color = engine_draw.black)
        self.y_tween = Tween()
        self.y_tween.start(self.y_sprite, "position", self.y_sprite.position, Vector2(-4, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)


        self.C_texture = TextureResource("C.bmp")
        self.C_sprite = Sprite2DNode(texture=self.C_texture, position=Vector2(10, -100), transparent_color = engine_draw.black)
        self.C_tween = Tween()
        self.C_tween.start(self.C_sprite, "position", self.C_sprite.position, Vector2(10, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.GO_texture = TextureResource("GO.bmp")
        self.GO_sprite = Sprite2DNode(texture=self.GO_texture, position=Vector2(21, -100), transparent_color = engine_draw.black)
        self.GO_tween = Tween()
        self.GO_tween.start(self.GO_sprite, "position", self.GO_sprite.position, Vector2(21, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.L_texture = TextureResource("L.bmp")
        self.L_sprite = Sprite2DNode(texture=self.L_texture, position=Vector2(31, -100), transparent_color = engine_draw.black)
        self.L_tween = Tween()
        self.L_tween.start(self.L_sprite, "position", self.L_sprite.position, Vector2(31, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.RO_texture = TextureResource("RO.bmp")
        self.RO_sprite = Sprite2DNode(texture=self.RO_texture, position=Vector2(41, -100), transparent_color = engine_draw.black)
        self.RO_tween = Tween()
        self.RO_tween.start(self.RO_sprite, "position", self.RO_sprite.position, Vector2(41, 0), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)

        self.R_texture = TextureResource("R.bmp")
        self.R_sprite = Sprite2DNode(texture=self.R_texture, position=Vector2(52, -100), transparent_color = engine_draw.black)
        self.R_tween = Tween()
        self.R_tween.start(self.R_sprite, "position", self.R_sprite.position, Vector2(52, -1), random.uniform(2000.0, 2400.0), 1.0, ONE_SHOT, EASE_ELAST_OUT)


delay = MainDelay()
delay.start(1000.0)

camera = CameraNode()

# engine.start()
while True:
    engine.tick()