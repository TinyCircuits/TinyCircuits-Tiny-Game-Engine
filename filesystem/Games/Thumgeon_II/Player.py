from engine_nodes import Sprite2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2

import Render
import Tiles
import math

player_texture = TextureResource("PlayerTile2_16bit.bmp")

apple_texture = TextureResource("items/Apple_16bit.bmp")
bomb_texture = TextureResource("items/Bomb-Sheet_16bit.bmp")
redbook_texture = TextureResource("items/Book1_16bit.bmp")
bluebook_texture = TextureResource("items/Book2_16bit.bmp")
bow_texture = TextureResource("items/Bow1-Sheet_16bit.bmp")
catbomb_texture = TextureResource("items/Catbomb-Sheet_16bit.bmp")
dagger_texture = TextureResource("items/Dagger_16bit.bmp")
goldcoins_texture = TextureResource("items/Gold-coins_16bit.bmp")
goldbars_texture = TextureResource("items/Gold-bars_16bit.bmp")
ham_texture = TextureResource("items/Ham_16bit.bmp")
honey_texture = TextureResource("items/Honeycomb_16bit.bmp")
clover_texture = TextureResource("items/Clovers_16bit.bmp")
bluepot_texture = TextureResource("items/PotionBlue_16bit.bmp")
redpot_texture = TextureResource("items/PotionRed_16bit.bmp")
rune1_texture = TextureResource("items/Rune1_16bit.bmp")
rune2_texture = TextureResource("items/Rune2_16bit.bmp")
rune3_texture = TextureResource("items/Rune3_16bit.bmp")
rune4_texture = TextureResource("items/Rune4_16bit.bmp")
rune5_texture = TextureResource("items/Rune5_16bit.bmp")
rune6_texture = TextureResource("items/Rune6_16bit.bmp")
rune7_texture = TextureResource("items/Rune7_16bit.bmp")
rune8_texture = TextureResource("items/Rune8_16bit.bmp")
sword_texture = TextureResource("items/Sword_16bit.bmp")

frame_texture = TextureResource("items/ItemFrame_16bit.bmp")

MAX_WEIGHT = 30

MAX_HP = 20
MAX_MP = 20
MAX_WT = 30

item_ids = {
    "none": 0,
    "apple": 1,
    "bomb": 2,
    "red_book": 3,
    "blue_book": 4,
    "bow": 5,
    "cat_bomb": 6,
    "dagger": 7,
    "gold_coins": 8,
    "gold_bars": 9,
    "ham": 10,
    "honey": 11,
    "clover": 12,
    "blue_pot": 13,
    "red_pot": 14,
    "rune1": 15,
    "rune2": 16,
    "rune3": 17,
    "rune4": 18,
    "rune5": 19,
    "rune6": 20,
    "rune7": 21,
    "rune8": 22,
    "sword": 23,
}

item_qualities = {
    "broken": 0,
    "basic": 1,
    "good": 2,
    "epic": 3,
    "ultra": 4,
}

spell_ids = {
    "novice_spark": 0,
    "intermediate_spark": 1,
    "advanced_spark": 2,
    "novice_confusion": 3,
    "intermediate_confusion": 4,
    "advanced_confusion": 5,
    "novice_teleport": 6,
    "intermediate_teleport": 7,
    "advanced_teleport": 8,
    "novice_firebolt": 9,
    "intermediate_firebolt": 10,
    "advanced_firebolt": 11,
    "novice_heal": 12,
    "intermediate_heal": 13,
    "advanced_heal": 14,
    "mage_push": 15,
    "novice_leech": 16,
    "intermediate_leech": 17,
    "advanced_leech": 18,
}

item_textures = {
    item_ids["none"]: None,
    item_ids["apple"]: apple_texture,
    item_ids["bomb"]: bomb_texture,
    item_ids["red_book"]: redbook_texture,
    item_ids["blue_book"]: bluebook_texture,
    item_ids["bow"]: bow_texture,
    item_ids["cat_bomb"]: catbomb_texture,
    item_ids["dagger"]: dagger_texture,
    item_ids["gold_coins"]: goldcoins_texture,
    item_ids["gold_bars"]: goldbars_texture,
    item_ids["ham"]: ham_texture,
    item_ids["honey"]: honey_texture,
    item_ids["clover"]: clover_texture,
    item_ids["blue_pot"]: bluepot_texture,
    item_ids["red_pot"]: redpot_texture,
    item_ids["rune1"]: rune1_texture,
    item_ids["rune2"]: rune2_texture,
    item_ids["rune3"]: rune3_texture,
    item_ids["rune4"]: rune4_texture,
    item_ids["rune5"]: rune5_texture,
    item_ids["rune6"]: rune6_texture,
    item_ids["rune7"]: rune7_texture,
    item_ids["rune8"]: rune8_texture,
    item_ids["sword"]: sword_texture,
}

item_weights = {
    item_ids["none"]: 0,
    item_ids["apple"]: 1,
    item_ids["bomb"]: 3,
    item_ids["red_book"]: 2,
    item_ids["blue_book"]: 2,
    item_ids["bow"]: 3,
    item_ids["cat_bomb"]: 3,
    item_ids["dagger"]: 3,
    item_ids["gold_coins"]: 0,
    item_ids["gold_bars"]: 0,
    item_ids["ham"]: 2,
    item_ids["honey"]: 2,
    item_ids["clover"]: 1,
    item_ids["blue_pot"]: 1,
    item_ids["red_pot"]: 1,
    item_ids["rune1"]: 0,
    item_ids["rune2"]: 0,
    item_ids["rune3"]: 0,
    item_ids["rune4"]: 0,
    item_ids["rune5"]: 0,
    item_ids["rune6"]: 0,
    item_ids["rune7"]: 0,
    item_ids["rune8"]: 0,
    item_ids["sword"]: 5,
}

item_frame_count = {
    item_ids["none"]: 0,
    item_ids["apple"]: 1,
    item_ids["bomb"]: 3,
    item_ids["red_book"]: 1,
    item_ids["blue_book"]: 1,
    item_ids["bow"]: 2,
    item_ids["cat_bomb"]: 3,
    item_ids["dagger"]: 1,
    item_ids["gold_coins"]: 1,
    item_ids["gold_bars"]: 1,
    item_ids["ham"]: 1,
    item_ids["honey"]: 1,
    item_ids["clover"]: 1,
    item_ids["blue_pot"]: 1,
    item_ids["red_pot"]: 1,
    item_ids["rune1"]: 1,
    item_ids["rune2"]: 1,
    item_ids["rune3"]: 1,
    item_ids["rune4"]: 1,
    item_ids["rune5"]: 1,
    item_ids["rune6"]: 1,
    item_ids["rune7"]: 1,
    item_ids["rune8"]: 1,
    item_ids["sword"]: 1,
}

hp_values = {
    item_ids["none"]: None,
    item_ids["apple"]: 6,
    item_ids["bomb"]: None,
    item_ids["red_book"]: None,
    item_ids["blue_book"]: None,
    item_ids["bow"]: None,
    item_ids["cat_bomb"]: None,
    item_ids["dagger"]: None,
    item_ids["gold_coins"]: None,
    item_ids["gold_bars"]: None,
    item_ids["ham"]: 12,
    item_ids["honey"]: 10,
    item_ids["clover"]: None,
    item_ids["blue_pot"]: None,
    item_ids["red_pot"]: 20,
    item_ids["rune1"]: None,
    item_ids["rune2"]: None,
    item_ids["rune3"]: None,
    item_ids["rune4"]: None,
    item_ids["rune5"]: None,
    item_ids["rune6"]: None,
    item_ids["rune7"]: None,
    item_ids["rune8"]: None,
    item_ids["sword"]: None,
}

dmg_values = {
    item_ids["none"]: None,
    item_ids["apple"]: None,
    item_ids["bomb"]: None,
    item_ids["red_book"]: None,
    item_ids["blue_book"]: None,
    item_ids["bow"]: 8,
    item_ids["cat_bomb"]: None,
    item_ids["dagger"]: 6,
    item_ids["gold_coins"]: None,
    item_ids["gold_bars"]: None,
    item_ids["ham"]: None,
    item_ids["honey"]: None,
    item_ids["clover"]: None,
    item_ids["blue_pot"]: None,
    item_ids["red_pot"]: None,
    item_ids["rune1"]: None,
    item_ids["rune2"]: None,
    item_ids["rune3"]: None,
    item_ids["rune4"]: None,
    item_ids["rune5"]: None,
    item_ids["rune6"]: None,
    item_ids["rune7"]: None,
    item_ids["rune8"]: None,
    item_ids["sword"]: 10,
}

class Item:
    def __init__(self, i = 0):
        self.id = i
        self.quality = -1
        self.use_func = None
        self.texture = item_textures[i]
        self.frame_count = item_frame_count[i]
        pass

class Inventory:
    def __init__(self):
        self.wt = 0
        self.max_wt = MAX_WEIGHT
        self.items = [None]
        
s_and_s_factor= 0.90

class Player(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = player_texture
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.set_layer(6)
        self.opacity = 1.2
        self.transparent_color = Color(0x07e0)
        self.width = 32
        self.height = 32
        self.inventory = []
        
        Render.cam.add_child(self)
        
        self.wt = 0
        self.maxwt = MAX_WT
        self.hp = MAX_HP
        self.mp = MAX_MP
        self.maxhp = MAX_HP
        self.maxmp = MAX_MP
        self.gp = 0
        self.held_item = Item(item_ids["bow"])
        
        self.time = 0
        
        self.look_ang = 0.0
        
        self.held_item_spr = Sprite2DNode()
        self.held_item_spr.frame_count_x = 2
        self.held_item_spr.playing = False
        self.held_item_spr.transparent_color  = Color(0x07e0)
        self.held_item_spr.set_layer(6)
        self.add_child(self.held_item_spr)
    
    def add_inv_item(self, item):
        if(item_weights[item] + self.wt <= self.maxwt):
            self.inventory.append(Item(item))
            self.wt += item_weights[item]
            return True
        else:
            return False
    
    def tick(self, dt):
        self.time += dt
        
        
        self.scale.x = 1.0
        self.held_item_spr.texture = self.held_item.texture
        self.held_item_spr.position = Vector2(8, 0)
        if(self.look_ang < -math.pi/2 or self.look_ang > math.pi/2):
            self.scale.x = -1.0
            self.held_item_spr.position = Vector2(-8, 0)
        self.held_item_spr.rotation = self.look_ang
        
        sint = math.sin(4*self.time)
        self.scale.x *= (s_and_s_factor + ((1-s_and_s_factor)/2)*(1-sint))
        self.scale.y *= (s_and_s_factor + ((1-s_and_s_factor)/2)*sint)
        
    
    
    
        