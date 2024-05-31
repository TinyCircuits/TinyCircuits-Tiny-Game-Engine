from engine_nodes import Sprite2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2

import math

skeleton_texture = TextureResource("monsters/Skeleton_16bit.bmp")
mimic_texture = TextureResource("monsters/Chest1Mimic-Sheet_16bit.bmp")
chupacabra_texture = TextureResource("monsters/Chupacabra_16bit.bmp")
ghost1_texture = TextureResource("monsters/Ghost1_16bit.bmp")
ghost2_texture = TextureResource("monsters/Ghost2_16bit.bmp")
scorpion_texture = TextureResource("monsters/Scorpion-Sheet_16bit.bmp")
slime_texture = TextureResource("monsters/Slime_16bit.bmp")

monster_ids = {
    "skeleton": 0,
    "mimic": 1,
    "chupacabra": 2,
    "ghost1": 3,
    "ghost2": 4,
    "scorpion": 5,
    "slime": 6,
}

monster_textures = {
       monster_ids["skeleton"]: skeleton_texture,
       monster_ids["mimic"]: mimic_texture,
       monster_ids["chupacabra"]: chupacabra_texture,
       monster_ids["ghost1"]: ghost1_texture,
       monster_ids["ghost2"]: ghost2_texture,
       monster_ids["scorpion"]: scorpion_texture,
       monster_ids["slime"]: slime_texture,
}

s_and_s_factor= 0.85

class Monster(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = skeleton_texture
        self.position = Vector2(5, 5)
        self.time = 0
    def tick(self, dt):
        #print("Monster dt is " + str(dt))
        self.time += dt
        sint = math.sin(4*self.time)
        self.scale = Vector2(s_and_s_factor + ((1-s_and_s_factor)/2)*(1-sint), s_and_s_factor + ((1-s_and_s_factor)/2)*sint)
        pass