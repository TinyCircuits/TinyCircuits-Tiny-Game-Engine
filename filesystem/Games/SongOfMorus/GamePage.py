
import engine_main

import random

import engine
import engine_io
import engine_draw
import engine_audio
import math
import engine_audio
from engine_draw import Color
from engine_nodes import CameraNode, Sprite2DNode, Text2DNode
from engine_resources import TextureResource,  WaveSoundResource, FontResource
from engine_math import Vector2,Vector3
from engine_animation import Tween, ONE_SHOT, EASE_BACK_IN, EASE_EXP_IN,EASE_EXP_OUT

import CustomMath
from GameObject import GameObject
from GameLevel import GameLevel




class GamePage:
    def __init__(self):
        self.loaded = False
        self.projection = [[1.6,0,0,0],[0,1.6,.6,0],[0,0,-1,-0.60018],[0,0,-1,0]]
        
    @micropython.native
    def preload(self):
        if(not self.loaded):
            self.loaded = True
            self.bg_road_tex = TextureResource("bg/road.bmp")
            self.bg_tex = TextureResource("bg/bg.bmp")
            self.building_blue_tex = TextureResource("bg/building_blue.bmp")
            self.building_red_tex = TextureResource("bg/building_red.bmp")
            self.song_run_tex = TextureResource("sprite/song_run.bmp")
            self.song_panic_tex = TextureResource("sprite/song_panic.bmp")
            self.song_faint_tex = TextureResource("sprite/song_faint.bmp")
            self.car_taxi_tex = TextureResource("sprite/car_taxi.bmp")
            self.fulu_tex = TextureResource("sprite/fulu.bmp",True)
            self.ui_heart_off_tex = TextureResource("sprite/ui_heart_off.bmp")
            self.ui_heart_on_tex = TextureResource("sprite/ui_heart_on.bmp")
            self.explosion_tex = TextureResource("sprite/explosion.bmp")
            self.hit_tex = TextureResource("sprite/hit.bmp")
            self.gameover_tex = TextureResource("sprite/gameover.bmp")
            self.roboto_font = FontResource("misc/9pt-roboto-font.bmp")

            self.ouch_sfx = WaveSoundResource("sound/ouch.wav")
            self.paper_sfx = WaveSoundResource("sound/paper.wav")
        
    @micropython.native
    def renderBG(self):
        pL = CustomMath.world_to_screen_point(Vector3(-2,0,.5),self.camX,self.camY,self.projection)
        pR = CustomMath.world_to_screen_point(Vector3(-2,0,9),self.camX,self.camY,self.projection)

        for i in range(int(pR.y+64),min( int(pL.y+64),128)):
            if(i&1): continue
            uv1 = CustomMath.screen_to_direction(Vector2(0,i),self.projection[1][1],self.projection[1][2])

            uv1.x= -self.camX + uv1.x/uv1.y*self.camY
            uv1.x/= 4 #road width
            uv1.x+= .5 #uv offset

            uv1.z= uv1.z/uv1.y*self.camY        
            uv1.z /= 4 
            
            uv2 = CustomMath.screen_to_direction(Vector2(127,i),self.projection[1][1],self.projection[1][2])
            uv2.x= -self.camX + uv2.x/uv2.y*self.camY
            uv2.x/= 4 #road width
            uv2.x+= .5 #uv offset

            
            len = uv2.x - uv1.x
            if(uv1.x<0):
                start = 0
            else:
                start = uv1.x
            if(uv2.x>1):
                end = 1
            else:
                end = uv2.x

            
            #if(test): 
            #    print(uv1.x/len,uv2.x,int(-uv1.x/len*128),int((1-uv1.x)/len*128),start,end)
            #    test = False
            
            if(uv2.x<1):
                uv2.x =1
            else:
                uv2.x =(1-uv1.x)/len
            if(uv1.x>0):
                uv1.x =0
            else:
                uv1.x =-uv1.x/len
            #self.renderBG_v(int(uv1.x*128),int(uv2.x*128),i,int(start*256*256),int(((end-start)/(uv2.x-uv1.x)*256)*2),int(((uv1.z+ self.roadScroll)%1)*256)) # 256 for point, 256 for length
            self.renderBG_v(int(uv1.x*128),int(uv2.x*128),i,int(start*256*256),int((end-start)/(uv2.x-uv1.x)*256*2),int(((uv1.z+ self.roadScroll)%1)*256)) # 256 for point, 256 for length



    @micropython.viper
    def renderBG_v(self,x1:int,x2:int,y:int,start:int,mul:int,z:int):
        buf = ptr16(engine_draw.back_fb_data())
        tex = ptr16(self.bg_road_tex.data)
        y2 = (y+1)<<7
        y = y<<7

        uv = start
        for x in range(x1, x2):
            buf[x|y] = buf[x|y2] = tex[(uv>>8 )&255 | (z<<8)]
            uv += mul


    @micropython.native
    def calGameObject(self,gameObj):
        pos = CustomMath.world_to_screen_point(gameObj.point,self.camX,self.camY,self.projection)
        gameObj.node.position.x = pos.x 
        gameObj.node.position.y = pos.y +gameObj.offsetY/ pos.z
        if(gameObj.scale!=0):
            gameObj.node.scale.x = gameObj.node.scale.y = gameObj.scale/ pos.z
        if(gameObj.flipped):
            gameObj.node.scale.x = -gameObj.node.scale.x
        if(gameObj.layer):
            gameObj.node.layer =(int(125/(1+gameObj.point.z)))
        if(gameObj.shadow):
            gameObj.shadow.scale = gameObj.node.scale
           #gameObj.shadow.position = Vector2(gameObj.node.position.x,gameObj.node.position.y+(1+gameObj.point.y)*pos.z )
           #gameObj.shadow.position = Vector2(gameObj.node.position.x,gameObj.node.position.y+(gameObj.point.y)*pos.z )
            pos = CustomMath.world_to_screen_point(Vector3(gameObj.point.x,0,gameObj.point.z),self.camX,self.camY,self.projection)
            gameObj.shadow.position.x = pos.x 
            gameObj.shadow.position.y = pos.y 

    @micropython.native
    def reduceHeart(self):
        engine_io.rumble(.3)
        engine_audio.play(self.ouch_sfx, 1, False)
        self.hearts -=1
        for j in range(0, len(self.uiHearts)):
            if(j<self.hearts):
                self.uiHearts[j].texture=self.ui_heart_on_tex
            else:
                self.uiHearts[j].texture=self.ui_heart_off_tex



    #@micropython.native
    def start(self):
        engine_draw. set_background(self.bg_tex)
        cam = CameraNode()
        self.camX = 0
        self.camY = .9

        self.hearts = 5
        score = 0
        time = 0
        invincible = 0
        self.roadScroll = 0
        buildingScroll = 0
        buildingToggle = True

        effects = []
        bullets = []
        bulletTimer =0
        self.attacks = []
        self.enemies = []
        self.obstacles = []
        self.uiHearts = [0 for x in range(self.hearts)]
        for j in range(0, self.hearts):  
            self.uiHearts[j] = Sprite2DNode(texture=self.ui_heart_on_tex,transparent_color=Color(0b0000011111100000),layer=127,position=Vector2(j*12-56,-56))
        score_text = Text2DNode(font=self.roboto_font,position = Vector2(45,-56),layer=127)
        score_text.text = '{0:06d}'.format(int(score))
       
        buildings =  [0 for x in range(12)]
        for j in range(0, 12):  
            if(j&2):
                buildings[j] = Sprite2DNode(texture=self.building_blue_tex , transparent_color=Color(0b0000011111100000))
            else:
                buildings[j] = Sprite2DNode(texture=self.building_red_tex , transparent_color=Color(0b0000011111100000))
                                
        song = GameObject(Sprite2DNode(texture=self.song_run_tex,frame_count_x=6,fps=15, transparent_color=Color(0b0000011111100000)),Vector3(0,0,1),-22,1,True,False )
        
        song.left = .1
        song.right = .1
        song.top = .3
        song.bottom = 0
        gameLevel = GameLevel()
        #game
        while True:
            if engine.tick():
                time+=1
                speed = gameLevel.getSpeed()
                if engine_io.RIGHT.is_pressed:
                    self.camX-=0.1
                    if(self.camX<-1.35): self.camX=-1.35
                elif engine_io.LEFT.is_pressed:
                    self.camX+=0.1
                    if(self.camX>1.35): self.camX=1.35
                if engine_io.UP.is_pressed:
                    self.camY+=0.05
                    if(self.camY>2): self.camY=2
                elif engine_io.DOWN.is_pressed:
                    self.camY-=0.05
                    if(self.camY<0.9): self.camY=0.9
                self.projection[1][2] = .6-(self.camY-0.9)*0.2
                song.point.x = (self.camX)*1.3
                song.point.y = (self.camY-0.9)*1.7
                if(time<30):
                    song.point.z = 0.1+time/30*0.9
                else:
                    song.point.z = 1
                if(invincible >0):
                    invincible-= 1
                    if(invincible ==60):
                        engine_io.rumble(0)
                        song.node.texture=self.song_run_tex
                        song.node.frame_count_x=6
                    if(invincible ==0):
                        song.node.opacity = 1
                    else:
                        song.node.opacity = 0.4+(invincible%2)*0.4
                if(self.hearts<=0):
                    break
                #bg
                self.roadScroll+=0.02
                if(self.roadScroll>1):
                   self.roadScroll -= 1
                buildingScroll +=speed
                if(buildingScroll>1.8):
                    buildingScroll-= 1.8
                    buildingToggle = not buildingToggle
                    for j in range(0, 12):  
                        if(buildingToggle):
                            if(j&2):
                                buildings[j] .texture=self.building_blue_tex 
                            else:
                                buildings[j] .texture=self.building_red_tex 
                        else:
                            if(j&2):
                                buildings[j] .texture=self.building_red_tex 
                            else:
                                buildings[j] .texture=self.building_blue_tex 
                self.renderBG()
                for i in range(0, 12):  
                    pos = 0
                    if(i&1):
                        pos = Vector3(2.4,0,9.5-math.floor(i/2)*1.8-buildingScroll)
                    else:
                        pos = Vector3(-2.4,0,9.5-math.floor(i/2)*1.8-buildingScroll)
                    pos = CustomMath.world_to_screen_point(pos,self.camX,self.camY,self.projection)
                    if(pos.z<1):
                        buildings[i].scale.x = buildings[i].scale.y =0 
                    else:
                        buildings[i].position.x = pos.x
                        buildings[i].position.y = pos.y -240/ pos.z
                        buildings[i].scale.y = 4/ pos.z
                        if(i&1):
                            buildings[i].scale.x = -buildings[i].scale.y 
                        else:
                            buildings[i].scale.x = buildings[i].scale.y 
                #
                gameLevel.update(self)
                newBullets = []
                newEnemies = []
                for enemy in self.enemies:
                    enemy.update(self,speed)
                    for bullet in bullets:
                        if(bullet.hp<100 and enemy.hitTest(bullet)):
                            bullet.hp = 999

                            
                            engine_audio.play(self.paper_sfx, 2, False)
                            enemy.hp-= 1
                            if enemy.hp <= 0:
                                score +=enemy.score
                                score_text.text = '{0:06d}'.format(int(score))
                                effect = GameObject( Sprite2DNode(texture=self.explosion_tex,frame_count_x=6,fps=12, transparent_color=Color(0b0000011111100000)),
                                    Vector3(enemy.point.x,enemy.point.y+0.24,enemy.point.z),
                                        0,2,True,False )
                                effect.hp = 12
                                effects.append(effect )
                            else:
                                effect = GameObject( Sprite2DNode(texture=self.hit_tex, transparent_color=Color(0b0000011111100000)),
                                    Vector3(bullet.point.x,bullet.point.y,bullet.point.z-0.05),
                                        0,2,True,False )
                                effect.hp = 3
                                effects.append( effect)
                            break
                        
                    if(invincible == 0 and song.hitTest(enemy)):
                        song.node.texture = self.song_panic_tex
                        song.node.frame_count_x = 1
                        invincible = 90
                        self.reduceHeart()
                    if(enemy.point.z <0.2 or enemy.hp <= 0):

                        enemy.destroy()
                    else:
                        self.calGameObject(enemy)
                        newEnemies.append(enemy)
                
                for bullet in bullets:
                    bullet.hp+=1
                    bullet.node.rotation = bullet.hp
                    bullet.point.z += 0.5
                    if(bullet.hp<24):
                        self.calGameObject(bullet)
                        newBullets.append(bullet)
                    else:
                        bullet.destroy()
                        
                if(time>30):
                    bulletTimer  += 1
                if(bulletTimer>4):
                    bullet = GameObject(Sprite2DNode(texture=self.fulu_tex, transparent_color=Color(0b0000011111100000)),Vector3(song.point.x,song.point.y+0.24,song.point.z),0,2,True,False )
                    newBullets.append(bullet)
                    bullet.node.rotation = time
                    bullet.point.z += 0.3
                    bullet.left = .4
                    bullet.right = .4
                    bullet.top = .4
                    bullet.bottom = .4
                    self.calGameObject(bullet)
                    bulletTimer = 0
                self.enemies = newEnemies
                bullets = newBullets
                newObstacles = []
                for obstacle in self.obstacles:
                    obstacle.update(self,speed)
                    if(invincible == 0 and song.hitTest(obstacle)):
                        song.node.texture = self.song_panic_tex
                        song.node.frame_count_x = 1
                        invincible = 90
                        self.reduceHeart()

                    if(obstacle.point.z <0.2):
                        obstacle.destroy()
                    else:
                        self.calGameObject(obstacle)
                        newObstacles.append(obstacle)
                self.obstacles = newObstacles
                newAttacks = []
                for attack in self.attacks:
                    attack.update(self,speed)
                    if(invincible == 0 and song.hitTest(attack)):
                        song.node.texture = self.song_panic_tex
                        song.node.frame_count_x = 1
                        invincible = 90
                        self.reduceHeart()
                        attack.destroy()
                    
                    elif(attack.point.z <0.2):
                        attack.destroy()
                    else:
                        self.calGameObject(attack)
                        newAttacks.append(attack)
                self.attacks = newAttacks
                newEffects = []
                for effect in effects:
                    effect.update(self,speed)
                    effect .hp-=1
                    if(effect.point.z <0.2 or effect.hp<=0):
                        effect.destroy()
                    else:
                        self.calGameObject(effect)
                        newEffects.append(effect)
                effects = newEffects
                self.calGameObject(song)
        #gameover
        self.renderBG() # the hp check break before rendering bg
        song.node.texture = self.song_faint_tex
        song.node.opacity = 1

        gameoverTime =0  
        song_tween = Tween()
        song_tween.start(song.node, "scale", Vector2(1,1),  Vector2(2,2), 500, .4, ONE_SHOT, EASE_EXP_OUT)
        song_tween2 = Tween()
        song_tween2.start(song.node, "position", song.node.position,  Vector2(song.node.position.x,120), 400, .4, ONE_SHOT, EASE_BACK_IN)
        song.node.layer =126
        while True:
            if engine.tick():
                gameoverTime+=1
                self.renderBG()
                song.node.rotation += .3
                
                if(gameoverTime==30):
                    engine_io.rumble(0)
                if(gameoverTime>60):
                    break
        
        gameover = Sprite2DNode(texture=self.gameover_tex,layer=127,position=Vector2(0,0), transparent_color=Color(0b0000011111100000))
        song.destroy()
        
        gameoverTime =0  
        while True:
            if engine.tick():
                gameoverTime+=1
                self.renderBG()
                if(gameoverTime>90):
                    break
        #remove all
        score_text.mark_destroy()
        gameover.mark_destroy()
        for enemy in self.enemies:
            enemy.destroy()
        for attack in self.attacks:
            attack.destroy()
        for bullet in bullets:
            bullet.destroy()
        for obstacle in self.obstacles:
            obstacle.destroy()
        for effect in effects:
            effect.destroy()
        for building in buildings:
            building.mark_destroy()
        for uiHeart in self.uiHearts:
            uiHeart.mark_destroy()  
        return 1
            
        
                
      

                

