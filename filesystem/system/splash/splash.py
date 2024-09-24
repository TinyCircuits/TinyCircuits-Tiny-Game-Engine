import engine
import engine_draw
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource
from engine_animation import Tween, Delay, ONE_SHOT, EASE_ELAST_OUT
from engine_math import Vector2
import random
import time

engine.fps_limit(60)

# Camera to render scene
camera = CameraNode()

def splash():
    T_texture = TextureResource("/system/splash/assets/T.bmp")
    T_sprite = Sprite2DNode(texture=T_texture, position=Vector2(-53, -100), transparent_color = engine_draw.black)
    T_tween = Tween()
    h_texture = TextureResource("/system/splash/assets/h.bmp")
    h_sprite = Sprite2DNode(texture=h_texture, position=Vector2(-44, -100), transparent_color = engine_draw.black)
    h_tween = Tween()
    u_texture = TextureResource("/system/splash/assets/u.bmp")
    u_sprite = Sprite2DNode(texture=u_texture, position=Vector2(-34, -100), transparent_color = engine_draw.black)
    u_tween = Tween()
    m_texture = TextureResource("/system/splash/assets/m.bmp")
    m_sprite = Sprite2DNode(texture=m_texture, position=Vector2(-24, -100), transparent_color = engine_draw.black)
    m_tween = Tween()
    b_texture = TextureResource("/system/splash/assets/b.bmp")
    b_sprite = Sprite2DNode(texture=b_texture, position=Vector2(-13, -100), transparent_color = engine_draw.black)
    b_tween = Tween()
    y_texture = TextureResource("/system/splash/assets/y.bmp")
    y_sprite = Sprite2DNode(texture=y_texture, position=Vector2(-4, -100), transparent_color = engine_draw.black)
    y_tween = Tween()
    C_texture = TextureResource("/system/splash/assets/C.bmp")
    C_sprite = Sprite2DNode(texture=C_texture, position=Vector2(10, -100), transparent_color = engine_draw.black)
    C_tween = Tween()
    GO_texture = TextureResource("/system/splash/assets/GO.bmp")
    GO_sprite = Sprite2DNode(texture=GO_texture, position=Vector2(21, -100), transparent_color = engine_draw.black)
    GO_tween = Tween()
    L_texture = TextureResource("/system/splash/assets/L.bmp")
    L_sprite = Sprite2DNode(texture=L_texture, position=Vector2(31, -100), transparent_color = engine_draw.black)
    L_tween = Tween()
    RO_texture = TextureResource("/system/splash/assets/RO.bmp")
    RO_sprite = Sprite2DNode(texture=RO_texture, position=Vector2(41, -100), transparent_color = engine_draw.black)
    RO_tween = Tween()
    R_texture = TextureResource("/system/splash/assets/R.bmp")
    R_sprite = Sprite2DNode(texture=R_texture, position=Vector2(52, -100), transparent_color = engine_draw.black)
    R_tween = Tween()

    time_min = 1800
    time_max = 2200
    T_tween.start(T_sprite, "position", None, Vector2(-53, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    h_tween.start(h_sprite, "position", None, Vector2(-44, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    u_tween.start(u_sprite, "position", None, Vector2(-34, 2), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    m_tween.start(m_sprite, "position", None, Vector2(-24, 2), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    b_tween.start(b_sprite, "position", None, Vector2(-13, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    y_tween.start(y_sprite, "position", None, Vector2(-4, 4), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    C_tween.start(C_sprite, "position", None, Vector2(10, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    GO_tween.start(GO_sprite, "position", None, Vector2(21, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    L_tween.start(L_sprite, "position", None, Vector2(31, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    RO_tween.start(RO_sprite, "position", None, Vector2(41, 0), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)
    R_tween.start(R_sprite, "position", None, Vector2(52, -1), random.uniform(time_min, time_max), 1.0, ONE_SHOT, EASE_ELAST_OUT)

    t0 = time.ticks_ms()
    while True:
        if engine.tick():
            if time.ticks_diff(time.ticks_ms(), t0) >= time_max:
                break

    T_tween.pause()
    h_tween.pause()
    u_tween.pause()
    m_tween.pause()
    b_tween.pause()
    y_tween.pause()
    C_tween.pause()
    GO_tween.pause()
    L_tween.pause()
    RO_tween.pause()
    R_tween.pause()

splash()
