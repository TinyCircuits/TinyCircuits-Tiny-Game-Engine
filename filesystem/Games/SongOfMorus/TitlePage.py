import engine_main

import engine
import engine_io
import engine_draw
import engine_audio
from engine_nodes import CameraNode, Sprite2DNode, Rectangle2DNode
from engine_draw import Color
from engine_math import Vector2
from engine_resources import  TextureResource, WaveSoundResource
from engine_animation import Tween, Delay, ONE_SHOT, EASE_LINEAR, EASE_CIRC_OUT, EASE_CIRC_IN

import os
import random
from GradientBackground import  GradientBackground


class TitlePage:

    def __init__(self):
        self.loaded = False


    def tweenFinish(self,tween):
        self.end = True
    @micropython.native
    def preload(self):
        if(not self.loaded):
            self.loaded = True
            self.logo_tex = TextureResource("menu/logo.bmp")
            self.blur_tex = TextureResource("cutscene/blur.bmp")
            self.press_tex = TextureResource("menu/press.bmp")
            self.start_sfx = WaveSoundResource("sound/speak.wav")
    def start(self):

        
        rect_node = Rectangle2DNode(width=140,height=140,color=Color(1,1,1),layer=4)
        rect_tween = Tween()
        rect_tween.start(rect_node, "opacity", 1.0, 0.0, 1500, 2, ONE_SHOT, EASE_LINEAR)


        cam = CameraNode()

        
        blurs = [0 for x in range(16)]
        for x in range(0, 16):
            blur = Sprite2DNode(texture=self.blur_tex,transparent_color=Color(0b0000011111100000),layer=0)

            blur.position = Vector2(random.randint(-64-32,64+32),random.randint(-64,64) )
            blur.opacity = random.random()*.5+.5
            blurs[x] = blur

        
        logo_node = Sprite2DNode(texture=self.logo_tex, transparent_color=Color(0b0000011111100000),layer=2)
        logo_node.position = Vector2(0,-10)
        press_node = Sprite2DNode(texture=self.press_tex, transparent_color=Color(0b0000011111100000),layer=2)
        press_node.position = Vector2(0,40)


        bg_frame = 0
        pressed = False
        self.end = False
        
        while True:
            # Only execute code as fast as the engine ticks (due to FPS limit)
            if engine.tick():
                bg_frame += 7
                if(bg_frame >= 512):
                    bg_frame -= 512
                if(bg_frame >= 256):
                    press_node.opacity = 1
                else:
                    press_node.opacity = 0
                GradientBackground.render(int(bg_frame))
                if(self.end ):
                    break
                if(not pressed):

                    if  engine_io.A.is_just_pressed or engine_io.B.is_just_pressed  :
                        engine_audio.play(self.start_sfx, 1, False)
                        pressed = True
                        rect_tween = Tween()
                        rect_tween.start(rect_node, "opacity", 0.0, 1.0, 1000, 2, ONE_SHOT, EASE_LINEAR)
                        rect_tween.after = self.tweenFinish
                for x in range(0, 16):
                    blurs[x].position.x += 20
                    if( blurs[x].position.x >96):
                        blurs[x].position.x -=128+64
                        blurs[x].position.y = random.randint(-64,64)
                        blurs[x].opacity = random.random()*.5+.5
                


        for x in range(0, 16):
            blurs[x].mark_destroy()
        logo_node.mark_destroy()  
        press_node.mark_destroy()
        return 1