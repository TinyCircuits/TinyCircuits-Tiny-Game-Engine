
from engine_nodes import  Sprite2DNode
from engine_math import Vector3
from engine_animation import Tween, ONE_SHOT, EASE_BACK_IN, EASE_LINEAR
from engine_resources import TextureResource
from engine_draw import Color

import random
import math


shadow_tex = TextureResource("sprite/char_shadow.bmp",True)

class GameObject:
    @micropython.native
    def __init__(self,node,point3D,offsetY,scale,layer,flipped):
        self.node = node
        self.point = point3D
        self.offsetY = offsetY
        self.scale = scale
        self.layer = layer
        self.flipped = flipped
        self.hp = 0
        #hitbox
        self.left = 0
        self.right = 0
        self.top = 0
        self.bottom = 0
        self.shadow = 0

    @micropython.native
    def addShadow(self):
        self.shadow = Sprite2DNode(texture=shadow_tex, transparent_color=Color(0b0000011111100000))
        self.shadow.opacity = 0.8

    @micropython.native
    def update(self,gamePage,offset):
        self.point.z -= offset

    @micropython.native
    def hitTest(self,other):
        
        return (
                (self.point.x-self.left<other.point.x+self.right) and
                 (self.point.x+self.right>other.point.x-self.left) and
                 (self.point.y-self.bottom<other.point.y+self.top) and
                 (self.point.y+self.top>other.point.y-self.bottom) and
                abs( self.point.z - other.point.z )<.4
                )

    @micropython.native
    def destroy(self):
        self.node.mark_destroy()
        if(self.shadow):
            self.shadow.mark_destroy()

class Enemy(GameObject):
    @micropython.native
    def __init__(self, node,point3D,offsetY,scale,layer,flipped):
        super().__init__(node,point3D,offsetY,scale,layer,flipped)
        self.score = 10

class HoppingJiangshi(Enemy):
    @micropython.native
    def __init__(self, node,point3D,offsetY,scale,layer,flipped):
        super().__init__(node,point3D,offsetY,scale,layer,flipped)
        self.time = random.randint(0,90)
        
    @micropython.native
    def update(self,gamePage,offset):
        super().update(gamePage,offset)
        self.time += 1
        if(self.time==100):
            self.time = 0
        if(self.time<50):
            self.point.y = self.time/50*2
        else:
            self.point.y = (100-self.time)/50*2


class Gingy(Enemy):
    @micropython.native
    def __init__(self, node,point3D,offsetY,scale,layer,flipped):
        super().__init__(node,point3D,offsetY,scale,layer,flipped)
        self.tween = 0
        self.hp = 30
        self.score = 300
        
    @micropython.native
    def update(self,gamePage,offset):
        if (self.tween == 0 or self.tween.finished ):
            self.tween = Tween() 
            newPoint =  Vector3(random.random()*2.8-1.4,random.random()*2,random.random()*3+3)
            temp = Vector3( self.point.x-newPoint.x,self.point.y-newPoint.y,self.point.z-newPoint.z)
            len = math.sqrt( temp.x*temp.x+temp.y*temp.y+temp.z*temp.z)
            self.tween.start(self, "point", self.point,  newPoint, 300*len, .4, ONE_SHOT, EASE_LINEAR)

class Bullet(GameObject):
    @micropython.native
    def __init__(self, node,point3D,offsetY,scale,layer,flipped):
        super().__init__(node,point3D,offsetY,scale,layer,flipped)
        self.velocity = Vector3(0,0,-.1)
        
    @micropython.native
    def update(self,gamePage,offset):
        #super().update(gamePage,offset)
        self.point.z -= .04
        self.point.x += self.velocity .x
        self.point.y += self.velocity .y
        self.point.z += self.velocity .z
        self.node.rotation += 0.5

