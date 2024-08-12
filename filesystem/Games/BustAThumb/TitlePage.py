import engine_main

import engine
import engine_io
import engine_draw
import engine_audio
from engine_resources import RTTTLSoundResource
from engine_nodes import CameraNode, Sprite2DNode,  EmptyNode, Text2DNode
from engine_draw import Color
from engine_animation import Tween, ONE_SHOT, EASE_ELAST_OUT, EASE_BACK_IN
from engine_math import Vector2
from engine_resources import FontResource,  TextureResource

import os
import random


class TitlePage:

    def __init__(self):
        0

    @micropython.viper
    def render(self,frame:int):
        buf = ptr16(engine_draw.back_fb_data())
        tex1 = ptr16(gradient1_tex.data)
        tex2 = ptr16(gradient2_tex.data)
        y7:int = 0
        yP:int = 0
        col1:int = 0
        col2:int = 0

        for py in range(0, 128):
            y7 = (py<<7)
            yP = (py+frame)
            col1 = tex1[py]
            col2 = tex2[py]
            for px in range(0, 128):
                if ((px & 16) == (yP & 16)):
                    buf[px|y7] = col1
                else :
                    buf[px|y7] = col2
    
    @micropython.native
    def updateBubble(self,bubbles):
        for x in range(0, 8):
            bubbles[x].position.y -= 2
            if(bubbles[x].position.y <-150):
                bubbles[x].position = Vector2(random.random()*140-70,random.random()*60+80)

    def start(self):
        global  bubble_tex
        global  gradient1_tex
        global  gradient2_tex
        global  logo_tex
        global  press_tex
        bubble_tex = TextureResource("tex/menu_bubble.bmp")
        gradient1_tex = TextureResource("tex/menu_gradient1.bmp")
        gradient2_tex = TextureResource("tex/menu_gradient2.bmp")
        logo_tex = TextureResource("tex/logo.bmp")
        press_tex = TextureResource("tex/press button.bmp")
        roboto_font = FontResource("tex/9pt-roboto-font.bmp")
        bgm = RTTTLSoundResource("music.rtttl")
        engine_audio.play(bgm, 1, True)

        cam = CameraNode()
        bubbles = [0 for x in range(8)]
        for x in range(0, 8):
            bubbles[x] = Sprite2DNode(texture=bubble_tex, transparent_color=Color(0b0000011111100000))
            bubbles[x].position = Vector2(random.random()*140-70,random.random()*180-90)
        logo_node = Sprite2DNode(texture=logo_tex, transparent_color=Color(0b0000011111100000))
        logo_node.layer =(1)
        logo_node.position = Vector2(0,-12)
        bg1_tween = Tween()
        bg1_tween.start(logo_node, "scale", Vector2(0,0),  Vector2(1,1), 600, .2, ONE_SHOT, EASE_ELAST_OUT)
        
        press_node = Sprite2DNode(texture=press_tex, transparent_color=Color(0b0000011111100000))
        press_node.layer =(1)
        press_node.position = Vector2(0,40)
        press_node.opacity  = 0
        
        credit_text = Text2DNode(font=roboto_font)
        credit_text.position = Vector2(0,58)
        credit_text.text = '@SunnyChowTheGuy'

        frame = 0
        wait = -60
        while True:
            # Only execute code as fast as the engine ticks (due to FPS limit)
            if engine.tick():
                self.render(int(frame))
                frame +=1
                if frame>=32: 
                    frame =0
                wait +=1
                if(wait>30):  wait = 0
                if(wait ==0 ): press_node.opacity  = 1
                if(wait ==15 ):  press_node.opacity  = 0
                if(wait>0): 
                    if  engine_io.A.is_just_pressed or engine_io.B.is_just_pressed  :
                        break
                self.updateBubble(bubbles)
                            
        bg1_tween.stop()
        bg1_tween.start(logo_node, "position", Vector2(0,-10),  Vector2(0,-120), 600, .6, ONE_SHOT, EASE_BACK_IN)
        wait = 0
        press_node.mark_destroy() 
        credit_text.mark_destroy()
        while True:
            if engine.tick():
                self.render(int(frame))
                frame +=1
                if frame>=32: 
                    frame =0
                wait +=1
                if(wait>50): break
                self.updateBubble(bubbles)

        engine_audio.stop(1)
        cam.mark_destroy()
        logo_node.mark_destroy()
        for x in range(0, 8):
            bubbles[x] .mark_destroy()
        return 1