
from engine_nodes import CameraNode, Sprite2DNode, Text2DNode
from engine_resources import TextureResource
from engine_math import Vector3
from engine_draw import Color

from GameObject import GameObject,HoppingJiangshi,Gingy,Bullet,Enemy
gingy_idle_tex = TextureResource("sprite/gingy_idle.bmp")
jiangshiA_idle_tex = TextureResource("sprite/jiangshiA_idle.bmp")
jiangshiB_idle_tex = TextureResource("sprite/jiangshiB_idle.bmp")
car_taxi_tex = TextureResource("sprite/car_taxi.bmp")
sign_h_tex = TextureResource("sprite/sign_h.bmp")
sign_pawnshop_tex = TextureResource("sprite/sign_pawnshop.bmp")
bullet_tex = TextureResource("sprite/bullet.bmp")



import random

class GameLevel:
    def __init__(self):
        self.time = 0
        self.enemyTime = 0
        self.obstacleTime = 0
        self.bulletTime = 0
        self.bulletPattern = 0
        self.bulletNumber = 0
        self.phase = 0
        self.gingy = 0
    @micropython.native
    def difficulty(self,start,end,current,last):
        p = current/last
        if(p>1): 
            p =1
        return start+(end-start)*p
    @micropython.native
    def gingyAttack(self):
        
        self.bulletPattern = random.randint(0,1)
        self.bulletTime = -15
        self.bulletNumber = 0
    @micropython.native
    def update(self,gamePage):
        if (self.phase%2==0):
            self.time+= 1
            enemyTimeMax= self.difficulty(50,15,self.phase,5)

            if(self.time >900):
                self.phase += 1
                self.gingy =  Gingy( Sprite2DNode(texture=gingy_idle_tex,frame_count_x=2,fps=12, transparent_color=Color(0b0000011111100000)),
                                        Vector3(0,0,7+1),
                                        -24,1.2,True,False )
                enemy = self.gingy
                enemy.left = .2
                enemy.right = .2
                enemy.top = .5
                enemy.bottom = .1
                enemy.addShadow()
                self.gingyAttack()
                gamePage.enemies.append(enemy )
            self.enemyTime+= 1
            if(self.enemyTime >enemyTimeMax):
                self.enemyTime -= enemyTimeMax
                if(random.random()<0.3):
                    enemy =  Enemy( Sprite2DNode(texture=jiangshiA_idle_tex,frame_count_x=2,fps=12, transparent_color=Color(0b0000011111100000)),
                                        Vector3(random.random()*3.6-1.8,0,7+1),
                                        -24,1.2,True,False )
                    enemy.left = .12
                    enemy.right = .12
                    enemy.top = .36
                    enemy.bottom = 0
                    enemy.hp = 3
                    gamePage.enemies.append(enemy )
                else:
                    enemy = HoppingJiangshi( Sprite2DNode(texture=jiangshiB_idle_tex,frame_count_x=2,fps=12, transparent_color=Color(0b0000011111100000)),
                                        Vector3(random.random
                                        ()*3.6-1.8,0,7+1),
                                        -24,1.2,True,False )
                    enemy.left = .12
                    enemy.right = .12
                    enemy.top = .36
                    enemy.bottom = 0
                    enemy.hp = 2
                    enemy.addShadow()
                    gamePage.enemies.append( enemy )
        elif (self.phase%2==1):
            if(self.gingy.hp<=0):
                self.time= 0
                self.phase +=1
            self.bulletTime+=1
            if(self.bulletPattern == 0):
                if(self.bulletTime> self.difficulty(15,6,self.phase-1,10)):
                    self.bulletTime = 0
                    bullet =  Bullet( Sprite2DNode(texture=bullet_tex,transparent_color=Color(0b0000011111100000)),
                                            Vector3(self.gingy.point.x,self.gingy.point.y+0.2,self.gingy.point.z-0.1),
                                            -24,1.2,True,False )
                    bullet.left = .12
                    bullet.right = .12
                    bullet.top = .36
                    bullet.bottom = 0
                    gamePage.attacks.append( bullet )
                    
                    self.bulletNumber+=1
                    if(self.bulletNumber>8):
                        self.gingyAttack()
            elif(self.bulletPattern == 1):
                if(self.bulletTime> self.difficulty(30,20,self.phase-1,10)):
                    for j in range(-4, 4): 
                            self.bulletTime = 0
                            bullet =  Bullet( Sprite2DNode(texture=bullet_tex,transparent_color=Color(0b0000011111100000)),
                                                        Vector3(self.gingy.point.x,self.gingy.point.y+0.2,self.gingy.point.z-0.1),
                                                        -24,1.2,True,False )
                            bullet.left = .12
                            bullet.right = .12
                            bullet.top = .36
                            bullet.bottom = 0
                            bullet.velocity.x = j*0.03
                            gamePage.attacks.append( bullet )
                    self.bulletNumber+=1
                    if(self.bulletNumber>4):
                        self.gingyAttack()

        #
        obstacleTimeMax=self.difficulty(84,25,self.phase,5)
        self.obstacleTime+= 1
        if(self.obstacleTime >obstacleTimeMax):
            self.obstacleTime -= obstacleTimeMax
            rnd = random.random()*3
            if(rnd<1):
                car =  GameObject( Sprite2DNode(texture=car_taxi_tex,transparent_color=Color(0b0000011111100000)),
                                            Vector3(random.random()*2.8-1.4,0,7+1),
                                            -12,3,True,random.random()<0.5 )
    
                car.left = .2
                car.right = .2
                car.top = .2
                car.bottom = 0
                gamePage.obstacles.append( car)
            elif(rnd<2):
                sign = GameObject( Sprite2DNode(texture=sign_h_tex,transparent_color=Color(0b0000011111100000)),
                                            Vector3((random.randint(0,1)*2-1)*1.2,random.random()*1+0.8,7+1),
                                            -12,3,True,False )
    
                sign.left = .4
                sign.right = .4
                sign.top = .15
                sign.bottom =.15
                gamePage.obstacles.append(sign  )
            elif(rnd<3):
                sign =  GameObject( Sprite2DNode(texture=sign_pawnshop_tex,transparent_color=Color(0b0000011111100000)),
                                            Vector3((random.randint(0,1)*2-1)*1.4,random.random()*1+0.8,7+1),
                                            -12,3,True,False )
    
                sign.left = .3
                sign.right = .3
                sign.top = .32
                sign.bottom =.32
                gamePage.obstacles.append( sign)
    @micropython.native
    def getSpeed(self):
        return self.difficulty(0.06,0.1,self.phase,10)


