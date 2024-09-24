import engine_main

import engine
import engine_io
import engine_draw
import engine_audio
from engine_nodes import CameraNode, Sprite2DNode, Rectangle2DNode
from engine_draw import Color
from engine_math import Vector2
from engine_resources import WaveSoundResource,  TextureResource
from engine_animation import Tween, Delay, ONE_SHOT, EASE_LINEAR, EASE_CIRC_OUT, EASE_CIRC_IN

import math
import random
from GradientBackground import  GradientBackground





class CutscenePage:

    def __init__(self):
        self.loaded = False
        self.loadedFirst = False
        self.tweenStep=0

    def tweenFinish(self,tween):
        if( self.cam == 0): 
             return
        self.tweenStep+=1

    
    @micropython.native
    def preload(self):
        if(not self.loaded):
            self.loaded = True
            self.blur_tex = TextureResource("cutscene/blur.bmp",True)
            self.gingy_tex = TextureResource("cutscene/gingy.bmp")
            self.song_final1_tex = TextureResource("cutscene/final_1.bmp")
            self.song_final2_tex = TextureResource("cutscene/final_2.bmp")
            self.fulu_tex = TextureResource("cutscene/fulu.bmp")
            self.song_texList = [0 for x in range(7)]
            for x in range(0, 7):
                self.song_texList[x] = TextureResource("cutscene/song_"+str(x)+".bmp")
    @micropython.native
    def firstFrame(self):
        if(not self.loadedFirst):
            self.loadedFirst = True
            self.credit_tex = TextureResource("cutscene/credit.bmp")
        self.credit_node = Sprite2DNode(texture=self.credit_tex,transparent_color=Color(0b0000011111100000),layer=1)
        self.cam = CameraNode()
        GradientBackground.render(int(0))
        while (not engine.tick()):
            0
    def start(self):
        GradientBackground.render(int(0))
        #

        self.music_sfx = WaveSoundResource("sound/music.wav")
        engine_audio.play(self.music_sfx, 1, False)
        
        self.tweenStep=0
        
        blurs = [0 for x in range(16)]
        for x in range(0, 16):
            blur = Sprite2DNode(texture=self.blur_tex,transparent_color=Color(0b0000011111100000),layer=0)

            blur.position = Vector2(random.randint(-64-32,64+32),random.randint(-64,64) )
            blur.opacity = random.random()*.5+.5
            blurs[x] = blur


        gingy_node = Sprite2DNode(texture=self.gingy_tex,transparent_color=Color(0b0000011111100000),layer=1)
        gingy_node.position = Vector2(128,0)
                
        self.song_x =Vector2(164,0)
        song_node = Sprite2DNode(texture=self.song_texList[0],transparent_color=Color(0b0000011111100000),layer=2)
        song_node.position.x = 200
        song_frame = 0
        bg_frame = 0
        frame = 0

        fulu_node = 0
        rect_node = 0

        while True:
            if engine.tick():
                if(self.tweenStep == 0):
                    def after(a):
                        if( self.cam == 0): 
                            return
                        credit_tween = Tween()
                        credit_tween.start(self.credit_node, "opacity", 1.0, 0.0, 600, 1, ONE_SHOT, EASE_LINEAR)
                        credit_tween.after = self.tweenFinish


                    credit_delay = Delay()
                    credit_delay.start(2000,after)
                    self.tweenStep += 1
                elif(self.tweenStep == 2):
                    gingy_tween = Tween()
                    gingy_tween.start(gingy_node, "position", gingy_node.position, Vector2(0,0), 600, 1, ONE_SHOT, EASE_CIRC_OUT)
                    gingy_tween.after = self.tweenFinish
                    self.tweenStep += 1
                elif(self.tweenStep == 4):
                    gingy_tween = Tween()
                    gingy_tween.start(gingy_node, "position", gingy_node.position, Vector2(-28,-30), 600, 1, ONE_SHOT, EASE_CIRC_OUT)
                    gingy_tween2 = Tween()
                    gingy_tween2.start(gingy_node, "scale", gingy_node.scale, Vector2(.5,.5), 600, 1, ONE_SHOT, EASE_CIRC_OUT)
                    song_tween = Tween()
                    song_tween.start(self, "song_x", self.song_x, Vector2(64,0), 500, 1, ONE_SHOT, EASE_CIRC_OUT)
                    gingy_tween.after = self.tweenFinish
                    self.tweenStep += 1
                elif(self.tweenStep == 6):
                    gingy_node.position = Vector2(-28,math.sin(frame*0.3)*3-30)
                    credit_delay = Delay()
                    credit_delay.start(2000,self.tweenFinish)
                    self.tweenStep += 1
                elif(self.tweenStep == 8):
                    gingy_tween = Tween()
                    gingy_tween.start(gingy_node, "position", gingy_node.position, Vector2(-200,gingy_node.position.y), 1500, 1, ONE_SHOT, EASE_CIRC_IN)
                    gingy_tween.after = self.tweenFinish
                    self.tweenStep += 1
                elif(self.tweenStep == 10):
                    for x in range(0, 16):

                        blurs[x].mark_destroy()
                        blurs[x] = 0
                    song_node.texture=self.song_final1_tex
                    credit_delay = Delay()
                    credit_delay.start(500,self.tweenFinish)
                    self.tweenStep += 1
                    song_node.position.x = 0
                    song_node.position.y = 0
                elif(self.tweenStep == 12):
                    song_node.texture=self.song_final2_tex
                    song_node.position.x = -6
                    song_node.position.y =18
                    song_tween = Tween()
                    song_tween.start(song_node, "position", song_node.position, Vector2(0,0), 1500, 2, ONE_SHOT, EASE_LINEAR)
                    song_tween2 = Tween()
                    song_tween2.start(song_node, "scale", song_node.scale, Vector2(.75,.75), 1500, 2, ONE_SHOT, EASE_LINEAR)
                    fulu_node = Sprite2DNode(texture=self.fulu_tex,transparent_color=Color(0b0000011111100000),layer=3)
                    fulu_tween = Tween()
                    fulu_tween.start(fulu_node, "scale", Vector2(.6,.6), Vector2(5,5), 1500, 2, ONE_SHOT, EASE_LINEAR)
                    fulu_tween2 = Tween()
                    fulu_tween2.start(fulu_node, "position", Vector2(20,-20), Vector2(0,0), 1500, 2, ONE_SHOT, EASE_LINEAR)
                    fulu_tween.after = self.tweenFinish
                    rect_node = Rectangle2DNode(width=140,height=140,color=Color(1,1,1),layer=4)
                    rect_tween = Tween()
                    rect_tween.start(rect_node, "opacity", 0.0, 1.0, 1500, 2, ONE_SHOT, EASE_LINEAR)

                    self.tweenStep += 1
                elif(self.tweenStep == 14):
                    break


                frame += 1
                bg_frame += 7
                if(bg_frame >= 512):
                    bg_frame -= 512
                GradientBackground.render(int(bg_frame))
                if(self.tweenStep == 7):
                    gingy_node.position = Vector2(-28,math.sin(frame*0.3)*3-30)
                if(self.tweenStep < 10):
                    for x in range(0, 16):
                        blurs[x].position.x += 20
                        if( blurs[x].position.x >96):
                            blurs[x].position.x -=128+64
                            blurs[x].position.y = random.randint(-64,64)
                            blurs[x].opacity = random.random()*.5+.5

                    song_frame+=1
                    if song_frame==14: 
                        song_frame =0
                    song_node.texture=self.song_texList[song_frame>>1]
                    song_node.position.x = self.song_x.x-song_node.texture.width/2
                    song_node.position.y = 64-song_node.texture.height/2
                if  engine_io.A.is_just_pressed or engine_io.B.is_just_pressed  :
                    break
        self.credit_node.mark_destroy()
        gingy_node.mark_destroy()
        song_node.mark_destroy()

        for x in range(0, 16):
            if( blurs[x]):
                blurs[x].mark_destroy()
        
        if(fulu_node): fulu_node.mark_destroy()
        if(rect_node): rect_node.mark_destroy()
        self.cam = 0
        return 1