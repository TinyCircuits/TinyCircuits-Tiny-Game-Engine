from engine_nodes import Sprite2DNode, Text2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2
from engine_animation import Tween

import math
import urandom
import Resources

skeleton_texture = TextureResource("monsters/Skeleton_16bit.bmp")
mimic_texture = TextureResource("monsters/Chest1Mimic-Sheet_16bit.bmp")
chupacabra_texture = TextureResource("monsters/Chupacabra_16bit.bmp")
ghost1_texture = TextureResource("monsters/Ghost1_16bit.bmp")
ghost2_texture = TextureResource("monsters/Ghost2_16bit.bmp")
scorpion_texture = TextureResource("monsters/Scorpion-Sheet_16bit.bmp")
slime_texture = TextureResource("monsters/Slime_16bit.bmp")
tempest_texture = TextureResource("monsters/Tempest_16bit.bmp")
snakedragon_texture = TextureResource("monsters/Snakeddragon_16bit.bmp")

monster_ids = {
    "skeleton": 1,
    "mimic": 2,
    "chupacabra": 3,
    "ghost1": 4,
    "ghost2": 5,
    "scorpion": 6,
    "slime": 7,
    "tempest": 8,
    "snakedragon": 9,
    "litbomb": 10,
}

monster_hp_range = {
    monster_ids["skeleton"]: (10, 12),
    monster_ids["mimic"]: (14, 18),
    monster_ids["chupacabra"]: (5, 8),
    monster_ids["ghost1"]: (4, 6),
    monster_ids["ghost2"]: (6, 10),
    monster_ids["scorpion"]: (3, 6),
    monster_ids["slime"]: (3, 7),
    monster_ids["tempest"]: (7, 10),
    monster_ids["snakedragon"]: (32, 40),
    monster_ids["litbomb"]: (-1)
}

monster_dmg_range = {
    monster_ids["skeleton"]: (3, 6),
    monster_ids["mimic"]: (7, 10),
    monster_ids["chupacabra"]: (2, 7),
    monster_ids["ghost1"]: (4, 6),
    monster_ids["ghost2"]: (6, 9),
    monster_ids["scorpion"]: (5, 7),
    monster_ids["slime"]: (2, 5),
    monster_ids["tempest"]: (4, 7),
    monster_ids["snakedragon"]: (12, 16),
    monster_ids["litbomb"]: (12, 16),
}

monster_textures = {
       monster_ids["skeleton"]: skeleton_texture,
       monster_ids["mimic"]: mimic_texture,
       monster_ids["chupacabra"]: chupacabra_texture,
       monster_ids["ghost1"]: ghost1_texture,
       monster_ids["ghost2"]: ghost2_texture,
       monster_ids["scorpion"]: scorpion_texture,
       monster_ids["slime"]: slime_texture,
       monster_ids["tempest"]: tempest_texture,
       monster_ids["snakedragon"]: snakedragon_texture,
       monster_ids["litbomb"]: Resources.bomb_texture,
}

monster_frame_count = {
       monster_ids["skeleton"]: 1,
       monster_ids["mimic"]: 2,
       monster_ids["chupacabra"]: 1,
       monster_ids["ghost1"]: 1,
       monster_ids["ghost2"]: 1,
       monster_ids["scorpion"]: 2,
       monster_ids["slime"]: 1,
       monster_ids["tempest"]: 1,
       monster_ids["snakedragon"]: 1,
       monster_ids["litbomb"]: 3,
}

s_and_s_factor= 0.85

class Monster(Sprite2DNode):
    def __init__(self, id = monster_ids["mimic"]):
        super().__init__(self)
        self.id = id
        self.texture = monster_textures[self.id]
        self.position = Vector2(4, 4)
        self.time = 0
        self.frame_count_x = monster_frame_count[self.id]
        self.frame_current_x = 0
        self.playing = False
        self.hp = urandom.randrange(monster_hp_range[self.id][0], monster_hp_range[self.id][1])
        self.hp_textnode = None
        self.stun_textnode = None
        self.layer = 5
        self.opacity = 0.0
        self.tween = Tween()
        self.stun = 5

    def set_monster(self, id):
        print("Setting monster to id " + str(id))
        self.time = 0
        self.id = id
        self.texture = monster_textures[id]
        print("Monster sprite dimensions: " + str(self.texture.width) + ", " + str(self.texture.height))
        print("Monster frames: " + str(monster_frame_count[id]))
        self.frame_count_x = monster_frame_count[id]
        print("Monster frames: " + str(self.frame_count_x))
        self.frame_current_x = 0
        self.frame_count_y = 1
        self.frame_current_y = 0

    def tick(self, dt):
        self.time += dt
        sint = math.sin(4*self.time)
        self.scale = Vector2(s_and_s_factor + ((1-s_and_s_factor)/2)*(1-sint), s_and_s_factor + ((1-s_and_s_factor)/2)*sint)
        if(self.hp_textnode is not None):
            self.hp_textnode.text = str(self.hp)
        if(self.id == monster_ids["litbomb"]):
            self.frame_current_x=1;
        if(self.stun == 0 and self.id == monster_ids["litbomb"]):
            self.frame_current_x=2;
