
import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
import engine_audio
from engine_draw import Color
from engine_nodes import CameraNode, Sprite2DNode,  EmptyNode, Text2DNode
from engine_resources import TextureResource,  FontResource, WaveSoundResource
from engine_math import Vector2
from engine_animation import Tween, ONE_SHOT, EASE_LINEAR, EASE_ELAST_OUT, EASE_SINE_IN
import os
import framebuf
import math
import engine_audio

class GamePage:
    def __init__(self):
        0
        
    @micropython.native
    def difficulty(self,start,end,current,last):
        p = current/last
        if(p>1): 
            p =1
        return start+(end-start)*p
    @micropython.viper
    def render(self,frame:int):
        buf = ptr16(engine_draw.back_fb_data())
        tex = ptr16(bg_tex.data)
        y6:int = 0
        y7:int = 0
        for py in range(4, 124):
            y6 = (py<<6)
            y7 = (py<<7)
            for px in range(6, 70):
                buf[px|y7] = tex[((px+frame)&63)|y6]
    @micropython.native
    def toGridPos(self,x,y):
        res = [0,round((y+56)/8)]
        if(res[1]%2 == self.firstRowOffset):
            res[0] = round((x+54)/8)
        else:
            res[0] = round((x+54-4)/8)
        return res
    @micropython.native
    def checkHeight(self):
        res = 0
        for py in range(0, 13):
            for px in range(0, 8):
                if( self.bubbles_data[py][px] != 0):
                    res = py
                    break
        return res
    @micropython.native
    def checkHit(self,x,y):
        return (x>=0 and x < 8 and y>=0 and y <= 12 and self.bubbles_data[y][x] !=0)
    @micropython.native
    def checkCombo(self,x,y,key):
        bubbles_node =self.bubbles_node
        bubbles_data =self.bubbles_data 
        bubbles_temp =self.bubbles_temp 
        wait = [[x,y]]
        combo = []
        bubble = bubbles_data[y][x]
        while(len(wait) >0):
            current = wait[0]
            wait.pop(0)
            if(current[0]<0 ) :
                continue
            if(current[0]>7 ) :
                continue
            if(current[1]<0 ) :
                continue
            if(current[1]>=13 ) :
                continue
            if(bubbles_temp[current[1]][current[0]] == key):
                continue
            if(bubbles_data[current[1]][current[0]] != bubble):
                continue
            combo.append(current)
            bubbles_temp[current[1]][current[0]] = key
            wait.append([current[0]-1,current[1]])
            wait.append([current[0]+1,current[1]])
            if(current[1]%2 ==  self.firstRowOffset ) :
                wait.append([current[0]-1,current[1]-1])
                wait.append([current[0],current[1]-1])
                wait.append([current[0]-1,current[1]+1])
                wait.append([current[0],current[1]+1])
            else:
                wait.append([current[0],current[1]-1])
                wait.append([current[0]+1,current[1]-1])
                wait.append([current[0],current[1]+1])
                wait.append([current[0]+1,current[1]+1])

        if(len(combo) >=3):
            for current in combo:
                bubbles_node[current[1]][current[0]].opacity = 0
                bubbles_data[current[1]][current[0]] = 0

            return combo
        return []
    @micropython.native
    def updateCommon(self):
        self.render(int((self.bgFrame%128)/2))
        self.bgFrame +=1
        bubbles_popping =self.bubbles_popping
        
        result = []
        for item in self.bubbles_falling:
            if (item[1].finished ):
                item[0].mark_destroy()
            else:
                result.append(item)
        self.bubbles_falling = result
        result = []
        for item in self.bubbles_popping:
            item[1] += 1
            if (item[1]>10 ):
                item[0].mark_destroy()
            else:
                result.append(item)
        self.bubbles_popping = result
            

    @micropython.native
    def checkGravity(self,key):
        bubbles_node =self.bubbles_node
        bubbles_data =self.bubbles_data 
        bubbles_temp =self.bubbles_temp 
        wait = []
        for px in range(0, 8):
            wait.append([px,0])
        while(len(wait) >0):
            current = wait[0]
            wait.pop(0)
            if(current[0]<0 ) :
                continue
            if(current[0]>7 ) :
                continue
            if(current[1]<0 ) :
                continue
            if(current[1]>=13 ) :
                continue
            if(bubbles_data[current[1]][current[0]] == 0):
                continue
            if(bubbles_temp[current[1]][current[0]] == key):
                continue
            bubbles_temp[current[1]][current[0]] = key
            wait.append([current[0]-1,current[1]])
            wait.append([current[0]+1,current[1]])
            if(current[1]%2 ==  self.firstRowOffset  ) :
                wait.append([current[0]-1,current[1]-1])
                wait.append([current[0],current[1]-1])
                wait.append([current[0]-1,current[1]+1])
                wait.append([current[0],current[1]+1])
            else:
                wait.append([current[0],current[1]-1])
                wait.append([current[0]+1,current[1]-1])
                wait.append([current[0],current[1]+1])
                wait.append([current[0]+1,current[1]+1])

        fall = []
        for py in range(0, 13):
            for px in range(0, 8):
                if(py%2 == 1 and px==7): continue
                if(bubbles_data[py][px] == 0):
                    continue
                if ( bubbles_temp[py][px] != key):
                    fall.append([px,py])
                    #bubbles_data[py][px] = 0
                    #bubbles_node[py][px].opacity = 0
        for pt in fall:
            b = Sprite2DNode(texture=bubble_tex[bubbles_data[pt[1]][pt[0]]], transparent_color=Color(0))
            bubbles_data[pt[1]][pt[0]] = 0
            bubbles_node[pt[1]][pt[0]].opacity = 0
            b.position =  bubbles_node[pt[1]][pt[0]].position
            b_tween = Tween()
            b_tween.start(b, "position", Vector2(b.position.x,b.position.y),  Vector2(b.position.x,b.position.y+80), 600, 1, ONE_SHOT, EASE_SINE_IN)
            self.bubbles_falling.append([b,b_tween])

        return fall
    def start(self):
        global bubble_tex
        global dead_bubble_tex 
        global char_idle_tex 
        global char_worry_tex 
        global char_happy_tex 
        global char_shock_tex 
        global char_cry_tex 
        global border_tex 
        global aim_tex 
        global bg_tex 
        global msg_gameover_tex 
        global msg_go_tex 
        global msg_ready_tex 

        roboto_font = FontResource("tex/9pt-roboto-font.bmp")
        bubble_tex = [0,TextureResource("tex/bubble_blue.bmp"),TextureResource("tex/bubble_green.bmp"),TextureResource("tex/bubble_purple.bmp"),TextureResource("tex/bubble_red.bmp"),TextureResource("tex/bubble_yellow.bmp"),TextureResource("tex/bubble_white.bmp")]
        dead_bubble_tex = TextureResource("tex/bubble_white.bmp")
        pop_bubble_tex = TextureResource("tex/bubble_pop.bmp")
        char_idle_tex = TextureResource("tex/thumbob_idle.bmp")
        char_worry_tex = TextureResource("tex/thumbob_worry.bmp")
        char_happy_tex = TextureResource("tex/thumbob_happy.bmp")
        char_shock_tex = TextureResource("tex/thumbob_shock.bmp")
        char_cry_tex = TextureResource("tex/thumbob_cry.bmp")
        border_tex = TextureResource("tex/border.bmp")
        aim_tex = TextureResource("tex/aim.bmp")
        bg_tex = TextureResource("tex/bg_loop.bmp")
        msg_gameover_tex = TextureResource("tex/msg_gameover.bmp")
        msg_go_tex = TextureResource("tex/msg_go.bmp")
        msg_ready_tex = TextureResource("tex/msg_ready.bmp")


        wave_ready = WaveSoundResource("ready.wav")
        wave_go = WaveSoundResource("go.wav")
        wave_pop = WaveSoundResource("pop.wav")
        wave_fail = WaveSoundResource("fail.wav")
        wave_fall = WaveSoundResource("fall.wav")


        cam = CameraNode()
        engine_draw. set_background(border_tex)
        bubbles_node =self.bubbles_node = [[0 for x in range(8)] for y in range(13)] 
        bubbles_data =self.bubbles_data = [[0 for x in range(8)] for y in range(13)] 
        bubbles_temp =self.bubbles_temp = [[0 for x in range(8)] for y in range(13)] 
        bubbles_falling = self.bubbles_falling= []
        bubbles_popping =self.bubbles_popping=  []

        for py in range(0, 13):
            for px in range(0, 8):
                b = bubbles_node[py][px] =  Sprite2DNode( transparent_color=Color(0))
                if(py%2 == 1 and px==7): 
                    bubbles_data[py][px] = 0
                elif(py<4):
                    bubbles_data[py][px] = random.randint(0,5)
                else:
                    bubbles_data[py][px] = 0
                if(bubbles_data[py][px] == 0):
                    b.opacity = 0
                else:
                    b.texture=bubble_tex[bubbles_data[py][px]]
                b.position.x = px*8-54 + (py%2)*4
                b.position.y = py*8-56
                b.layer =(1)
        character_node = Sprite2DNode(texture=char_idle_tex,frame_count_x=2,fps=3, transparent_color=Color(0b1111100000000000))
        character_node.position = Vector2(48,52)
        character_node.layer =(2)
        aim_node = Sprite2DNode(texture=aim_tex, transparent_color=Color(0b0000011111100000))
        aim_node.position = Vector2(-26,50)

        bubble_now = random.randint(1,5)
        bubble_next = random.randint(1,5)

        bubble_now_node = Sprite2DNode(texture=bubble_tex[bubble_now], transparent_color=Color(0))
        bubble_now_node.position = Vector2(-26,50)
        bubble_now_node.layer =(2)
        bubble_next_node = Sprite2DNode(texture=bubble_tex[bubble_next], transparent_color=Color(0))
        bubble_next_node.position = Vector2(25,47)
        bubble_next_node.layer =(2)

        score_text = Text2DNode(font=roboto_font)
        score_text.position = Vector2(36,-34)

        msg_node = Sprite2DNode(texture=msg_ready_tex, transparent_color=Color(0b0000011111100000))
        msg_node.layer =(2)
        

        score = 0
        score_text.text = '{0:06d}'.format(int(score))

        aimRotation = 0
        self.bgFrame =0
        fallTimer = 300
        shooting = False
        shootDir = Vector2(-1,1)
        mode = 1 
        self.firstRowOffset = 0

        engine_audio.play(wave_ready, 2, False)
        for i in range(1, 60):
            
            while (not engine.tick()):
                self.bgFrame
            self.updateCommon()
        msg_node.texture = msg_go_tex
        engine_audio.play(wave_go, 1, False)
        for i in range(1, 30):
            
            while (not engine.tick()):
                self.bgFrame
            self.updateCommon()
        msg_node.mark_destroy()

        while True:
            # Only execute code as fast as the engine ticks (due to FPS limit)
            if engine.tick():

                if engine_io.RIGHT.is_pressed:
                    if aimRotation>-1.4:
                            aimRotation-=0.06
                elif engine_io.LEFT.is_pressed:
                    if aimRotation<1.4:
                            aimRotation+=0.06
                if (shooting):
                    
                    for i in range(1, 3):
                        bubble_now_node.position.x += shootDir.x+shootDir.x
                        bubble_now_node.position.y += shootDir.y+shootDir.y

                        #edge
                        if(bubble_now_node.position.x<-54 and shootDir.x<0):
                            shootDir.x*=-1
                            bubble_now_node.position.x-=bubble_now_node.position.x+52
                        elif(bubble_now_node.position.x>0 and shootDir.x>0):
                            shootDir.x*=-1
                            bubble_now_node.position.x+=bubble_now_node.position.x-2
                        if(bubble_now_node.position.y<-52 and shootDir.y<0):
                            shootDir.y*=-1
                            bubble_now_node.position.y+=bubble_now_node.position.y+52
                        #bubble
                        hit = False
                        pt = self.toGridPos(bubble_now_node.position.x-2,bubble_now_node.position.y-3)
                        if(self.checkHit(pt[0],pt[1])) : hit = True
                        pt = self.toGridPos(bubble_now_node.position.x+2,bubble_now_node.position.y-3)
                        if(self.checkHit(pt[0],pt[1])) : hit = True
                        pt = self.toGridPos(bubble_now_node.position.x-2,bubble_now_node.position.y+3)
                        if(self.checkHit(pt[0],pt[1])) : hit = True
                        pt = self.toGridPos(bubble_now_node.position.x+2,bubble_now_node.position.y+3)
                        if(self.checkHit(pt[0],pt[1])) :  hit = True
                        pt = self.toGridPos(bubble_now_node.position.x+3,bubble_now_node.position.y)
                        if(self.checkHit(pt[0],pt[1])) : hit = True
                        pt = self.toGridPos(bubble_now_node.position.x-3,bubble_now_node.position.y)
                        if(self.checkHit(pt[0],pt[1])) :  hit = True


                        if(hit):
                            pt = self.toGridPos(bubble_now_node.position.x,bubble_now_node.position.y)
                            bubbles_data[pt[1]][pt[0]] = bubble_now
                            bubbles_node[pt[1]][pt[0]] .texture=bubble_tex[bubble_now]
                            bubbles_node[pt[1]][pt[0]] .opacity = 1
                            
                            shooting = False
                            bubble_now_node.position = Vector2(-26,50)
                            bubble_now = bubble_next
                            bubble_next = random.randint(1,5)
                            bubble_now_node.texture=bubble_tex[bubble_now]
                            bubble_next_node.texture=bubble_tex[bubble_next]
                            res = self.checkCombo(pt[0],pt[1],fallTimer)
                            for item in res:
                                pop =   Sprite2DNode( texture=pop_bubble_tex , transparent_color=Color(0))
                                pop.position = bubbles_node[item[1]][item[0]] .position
                                self.bubbles_popping.append([pop, 0])
                            score += len(res)*10
                            num = len(self.checkGravity(fallTimer+1))
                            score += num*(num+1)*10
                            score_text.text = '{0:06d}'.format(int(score))

                            height = self.checkHeight()
                            if(height>=12):
                                mode = 2
                                break
                            elif(num>0):
                                character_node.texture=char_happy_tex
                                engine_audio.play(wave_fall, 1, False)
                            elif(height>=8):
                                character_node.texture=char_worry_tex
                            else:
                                character_node.texture=char_idle_tex
                            if(height == 0):
                                fallTimer = 0

                                


                        else:

                            if(bubble_now_node.position.y>64):
                                shooting = False
                                bubble_now_node.position = Vector2(-26,50)
                                bubble_now = bubble_next
                                bubble_next = random.randint(1,5)

                                bubble_now_node.texture=bubble_tex[bubble_now]
                                bubble_next_node.texture=bubble_tex[bubble_next]
                else:
                    if engine_io.A.is_just_pressed:
                       engine_audio.play(wave_pop, 2, False)
                       shooting = True
                       shootDir.x = -math.sin(aimRotation)
                       shootDir.y = -math.cos(aimRotation)
                        
                aim_node.rotation =aimRotation
                self.updateCommon()
                if(fallTimer<90):
                    cam.position.x =self.bgFrame%2
                fallTimer -=1
                if(fallTimer<=0 and not shooting):
                    fallTimer = self.difficulty(240,160,score,20000)
                    self.firstRowOffset = 1- self.firstRowOffset
                    newRow = [0 for x in range(8)]
                    for px in range(0, 8):
                        newRow[px] = random.randint(0,5)

                    if( self.firstRowOffset == 1):
                        newRow[7] =0

                    bubbles_data.insert(0,newRow)
                    del  bubbles_data[13]
                    for py in range(0, 13):
                        for px in range(0, 8):
                            b = bubbles_node[py][px]
                            #if(py%2 == self.firstRowOffset and px==7): 
                            #    b.opacity = 0
                            #el
                            if(bubbles_data[py][px] == 0):
                                b.opacity = 0
                            else:
                                b.opacity = 1
                                b.texture=bubble_tex[bubbles_data[py][px]]
                            b.position.x = px*8-52 + ((py+self.firstRowOffset)%2)*4
                            b.position.y = py*8-56
                    num = len(self.checkGravity(fallTimer+self.bgFrame)) #TODO: optimize
                    height = self.checkHeight()
                    if(height>=12):
                        engine_audio.play(wave_fail, 2, False)
                        mode = 2
                        break
                    elif(num>0):
                        character_node.texture=char_happy_tex
                    elif(height>=8):
                        character_node.texture=char_worry_tex
                    else:
                        character_node.texture=char_idle_tex
                if mode == 2:
                    break


        msg_node = Sprite2DNode(texture=msg_gameover_tex, transparent_color=Color(0b0000011111100000))
        msg_node.layer =(2)

        character_node.texture=char_shock_tex
        character_node.frame_count_x =1
        engine_io.rumble(.4)
        for py in range(0, 13):
            for px in range(0, 8):
                bubbles_node[12-py][px].texture = dead_bubble_tex

            for i in range(1, 3):
                
                while (not engine.tick()):
                    i
                self.updateCommon()
        engine_io.rumble(0)

        character_node.texture=char_cry_tex
        character_node.frame_count_x =2
        character_node.fps=6

        for i in range(1, 120):
            
            while (not engine.tick()):
                i
            self.updateCommon()

        #
        for py in range(0, 13):
            for px in range(0, 8):
                b = bubbles_node[py][px].mark_destroy()

        aim_node.mark_destroy()
        character_node.mark_destroy()
        score_text.mark_destroy()
        bubble_now_node.mark_destroy()
        bubble_next_node .mark_destroy()
        msg_node.mark_destroy()

