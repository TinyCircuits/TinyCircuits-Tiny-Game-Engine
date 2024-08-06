from engine_nodes import Sprite2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2

import Resources
import Render
import Tiles
import math

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
    "rune_complete": 24
}

item_qualities = {
    "": 5,
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
    "lesser_mage_push": 15,
    "novice_leech": 16,
    "intermediate_leech": 17,
    "advanced_leech": 18,
    "greater_mage_push": 19,
}

blue_spells = (
    "novice_confusion",
    "intermediate_confusion",
    "advanced_confusion",
    "novice_teleport",
    "intermediate_teleport",
    "advanced_teleport",
    "novice_heal",
    "intermediate_heal",
    "advanced_heal",
    "lesser_mage_push",
    "greater_mage_push",
)

red_spells = (
    "novice_spark",
    "intermediate_spark",
    "advanced_spark",
    "novice_firebolt",
    "intermediate_firebolt",
    "advanced_firebolt",
    "novice_leech",
    "intermediate_leech",
    "advanced_leech",
)

item_textures = {
    item_ids["none"]: None,
    item_ids["apple"]: Resources.apple_texture,
    item_ids["bomb"]: Resources.bomb_texture,
    item_ids["red_book"]: Resources.redbook_texture,
    item_ids["blue_book"]: Resources.bluebook_texture,
    item_ids["bow"]: Resources.bow_texture,
    item_ids["cat_bomb"]: Resources.catbomb_texture,
    item_ids["dagger"]: Resources.dagger_texture,
    item_ids["gold_coins"]: Resources.goldcoins_texture,
    item_ids["gold_bars"]: Resources.goldbars_texture,
    item_ids["ham"]: Resources.ham_texture,
    item_ids["honey"]: Resources.honey_texture,
    item_ids["clover"]: Resources.clover_texture,
    item_ids["blue_pot"]: Resources.bluepot_texture,
    item_ids["red_pot"]: Resources.redpot_texture,
    item_ids["rune1"]: Resources.rune1_texture,
    item_ids["rune2"]: Resources.rune2_texture,
    item_ids["rune3"]: Resources.rune3_texture,
    item_ids["rune4"]: Resources.rune4_texture,
    item_ids["rune5"]: Resources.rune5_texture,
    item_ids["rune6"]: Resources.rune6_texture,
    item_ids["rune7"]: Resources.rune7_texture,
    item_ids["rune8"]: Resources.rune8_texture,
    item_ids["sword"]: Resources.sword_texture,
    item_ids["rune_complete"]: Resources.rune_complete_texture
}
# buy / sell price of item
item_prices = {
    item_ids["none"]: (-1, -1),
    item_ids["apple"]: (5, 3),
    item_ids["bomb"]: (10, 6),
    item_ids["red_book"]: (-1, -1),
    item_ids["blue_book"]: (-1, -1),
    item_ids["bow"]: (25, 16),
    item_ids["cat_bomb"]: (12, 7),
    item_ids["dagger"]: (18, 13),
    item_ids["gold_coins"]: (-1, -1),
    item_ids["gold_bars"]: (-1, -1),
    item_ids["ham"]: (12, 7),
    item_ids["honey"]: (10, 6),
    item_ids["clover"]: (-1, -1),
    item_ids["blue_pot"]: (20, 14),
    item_ids["red_pot"]: (30, 20),
    item_ids["rune1"]: (-1, -1),
    item_ids["rune2"]: (-1, -1),
    item_ids["rune3"]: (-1, -1),
    item_ids["rune4"]: (-1, -1),
    item_ids["rune5"]: (-1, -1),
    item_ids["rune6"]: (-1, -1),
    item_ids["rune7"]: (-1, -1),
    item_ids["rune8"]: (-1, -1),
    item_ids["sword"]: (24, 15),
    item_ids["rune_complete"]: (-1, -1),
}

quality_price_modifiers = {
    item_qualities[""]: 1.0,
    item_qualities["broken"]: 0.7,
    item_qualities["basic"]: 0.9,
    item_qualities["good"]: 1.2,
    item_qualities["epic"]: 1.5,
    item_qualities["ultra"]: 1.8,
}

# 0 means no animation
# 1 means rotate to face aim direction
# 2 means switch frame and face aim direction
item_animate = {
    item_ids["none"]: 0,
    item_ids["apple"]: 0,
    item_ids["bomb"]: 0,
    item_ids["red_book"]: 0,
    item_ids["blue_book"]: 0,
    item_ids["bow"]: 2,
    item_ids["cat_bomb"]: 0,
    item_ids["dagger"]: 1,
    item_ids["gold_coins"]: 0,
    item_ids["gold_bars"]: 0,
    item_ids["ham"]: 0,
    item_ids["honey"]: 0,
    item_ids["clover"]: 0,
    item_ids["blue_pot"]: 0,
    item_ids["red_pot"]: 0,
    item_ids["rune1"]: 0,
    item_ids["rune2"]: 0,
    item_ids["rune3"]: 0,
    item_ids["rune4"]: 0,
    item_ids["rune5"]: 0,
    item_ids["rune6"]: 0,
    item_ids["rune7"]: 0,
    item_ids["rune8"]: 0,
    item_ids["sword"]: 1,
    item_ids["rune_complete"]: 0
}

# 0 means no animation
# 1 means shake
# 2 means shake and change frame
item_tile_animate = {
    item_ids["none"]: 0,
    item_ids["apple"]: 1,
    item_ids["bomb"]: 1,
    item_ids["red_book"]: 0,
    item_ids["blue_book"]: 0,
    item_ids["bow"]: 0,
    item_ids["cat_bomb"]: 1,
    item_ids["dagger"]: 0,
    item_ids["gold_coins"]: 0,
    item_ids["gold_bars"]: 0,
    item_ids["ham"]: 1,
    item_ids["honey"]: 1,
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
    item_ids["sword"]: 1,
    item_ids["rune_complete"]: 0
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
    item_ids["rune_complete"]: 0,
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
    item_ids["rune_complete"]: 3,
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
    item_ids["rune_complete"]: None,
}

dmg_values = {
    item_ids["none"]: None,
    item_ids["apple"]: None,
    item_ids["bomb"]: None,
    item_ids["red_book"]: None,
    item_ids["blue_book"]: None,
    item_ids["bow"]: 6,
    item_ids["cat_bomb"]: None,
    item_ids["dagger"]: 4,
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
    item_ids["sword"]: 8,
    item_ids["rune_complete"]: None,
}

spell_mana_cost = {
    spell_ids["novice_spark"]: 2,
    spell_ids["intermediate_spark"]: 3,
    spell_ids["advanced_spark"]: 5,
    spell_ids["novice_firebolt"]: 5,
    spell_ids["intermediate_firebolt"]: 7,
    spell_ids["advanced_firebolt"]: 9,
    spell_ids["novice_leech"]: 5,
    spell_ids["intermediate_leech"]: 9,
    spell_ids["advanced_leech"]: 12,
    spell_ids["novice_confusion"]: 3,
    spell_ids["intermediate_confusion"]: 6,
    spell_ids["advanced_confusion"]: 10,
    spell_ids["novice_teleport"]: 8,
    spell_ids["intermediate_teleport"]: 12,
    spell_ids["advanced_teleport"]: 14,
    spell_ids["novice_heal"]: 6,
    spell_ids["intermediate_heal"]: 12,
    spell_ids["advanced_heal"]: 18,
    spell_ids["lesser_mage_push"]: 5,
    spell_ids["greater_mage_push"]: 8,
}

# buy / sell price
spell_prices = {
    spell_ids["novice_spark"]: (12, 7),
    spell_ids["intermediate_spark"]: (20, 14),
    spell_ids["advanced_spark"]: (30, 20),
    spell_ids["novice_firebolt"]: (12, 7),
    spell_ids["intermediate_firebolt"]: (20, 14),
    spell_ids["advanced_firebolt"]: (30, 20),
    spell_ids["novice_leech"]: (20, 14),
    spell_ids["intermediate_leech"]: (30, 20),
    spell_ids["advanced_leech"]: (45, 32),
    spell_ids["novice_confusion"]: (12, 7),
    spell_ids["intermediate_confusion"]: (20, 14),
    spell_ids["advanced_confusion"]: (30, 20),
    spell_ids["novice_teleport"]: (20, 14),
    spell_ids["intermediate_teleport"]: (30, 20),
    spell_ids["advanced_teleport"]: (45, 32),
    spell_ids["novice_heal"]: (20, 14),
    spell_ids["intermediate_heal"]: (30, 20),
    spell_ids["advanced_heal"]: (45, 32),
    spell_ids["lesser_mage_push"]: (12, 7),
    spell_ids["greater_mage_push"]:(20, 14),
}

spell_range = {
    spell_ids["novice_spark"]: 2,
    spell_ids["intermediate_spark"]: 2,
    spell_ids["advanced_spark"]: 2,
    spell_ids["novice_firebolt"]: 2,
    spell_ids["intermediate_firebolt"]: 2,
    spell_ids["advanced_firebolt"]: 2,
    spell_ids["novice_leech"]: 2,
    spell_ids["intermediate_leech"]: 2,
    spell_ids["advanced_leech"]: 2,
    spell_ids["novice_confusion"]: 2,
    spell_ids["intermediate_confusion"]: 2,
    spell_ids["advanced_confusion"]: 2,
    spell_ids["novice_teleport"]: 0,
    spell_ids["intermediate_teleport"]: 2,
    spell_ids["advanced_teleport"]: 3,
    spell_ids["novice_heal"]: 0,
    spell_ids["intermediate_heal"]: 0,
    spell_ids["advanced_heal"]: 2,
    spell_ids["lesser_mage_push"]: 1,
    spell_ids["greater_mage_push"]: 2,
}

spell_dmg_values = {
    spell_ids["novice_spark"]: 3,
    spell_ids["intermediate_spark"]: 5,
    spell_ids["advanced_spark"]: 8,
    spell_ids["novice_firebolt"]: 5,
    spell_ids["intermediate_firebolt"]: 8,
    spell_ids["advanced_firebolt"]: 11,
    spell_ids["novice_leech"]: 4,
    spell_ids["intermediate_leech"]: 6,
    spell_ids["advanced_leech"]: 8,
}

item_range = {
    item_ids["none"]: 1,
    item_ids["apple"]: 1,
    item_ids["bomb"]: 1,
    item_ids["red_book"]: 2,
    item_ids["blue_book"]: 2,
    item_ids["bow"]: 2,
    item_ids["cat_bomb"]: 1,
    item_ids["dagger"]: 1,
    item_ids["gold_coins"]: 0,
    item_ids["gold_bars"]: 0,
    item_ids["ham"]: 0,
    item_ids["honey"]: 0,
    item_ids["clover"]: 0,
    item_ids["blue_pot"]: 0,
    item_ids["red_pot"]: 0,
    item_ids["rune1"]: 0,
    item_ids["rune2"]: 0,
    item_ids["rune3"]: 0,
    item_ids["rune4"]: 0,
    item_ids["rune5"]: 0,
    item_ids["rune6"]: 0,
    item_ids["rune7"]: 0,
    item_ids["rune8"]: 0,
    item_ids["sword"]: 1,
    item_ids["rune_complete"]: 0,
}

class Item:
    def __init__(self, i = 0, d = 0):
        self.id = i
        self.quality = -1
        self.use_func = None
        self.texture = item_textures[i]
        self.frame_count = item_frame_count[i]
        if(i == item_ids["red_book"] or i == item_ids["blue_book"]):
            self.range = spell_range[d]
        else:
            self.range = item_range[i]
        self.data0 = d
        pass

class Inventory:
    def __init__(self):
        self.wt = 0
        self.max_wt = MAX_WEIGHT
        self.items = []

s_and_s_factor= 0.90

class Player(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Resources.player_texture
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.layer = 6
        self.opacity = 1.2
        self.transparent_color = Color(0x07e0)
        self.runes = 0
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
        #self.held_item = Item(item_ids["red_book"])
        self.held_item = None
        self.time = 0
        self.look_ang = 0.0

        # self.add_inv_item(item_ids["red_book"], spell_ids["advanced_leech"])
        self.add_inv_item(item_ids["blue_book"], spell_ids["novice_confusion"])
        self.add_inv_item(item_ids["dagger"], item_qualities["basic"])
        self.add_inv_item(item_ids["bomb"], item_qualities[""])

        self.held_item_spr = Sprite2DNode()
        self.held_item_spr.frame_count_x = 2
        self.held_item_spr.playing = False
        self.held_item_spr.transparent_color  = Color(0x07e0)
        self.held_item_spr.layer = 6
        self.held_item_spr.scale = Vector2(0.7, 0.7)
        self.add_child(self.held_item_spr)

    def set_held_item(self, item):
        self.held_item = item
        if(self.held_item_spr is None):
            self.held_item_spr = Sprite2DNode()
            self.held_item_spr.frame_count_x = 2
            self.held_item_spr.playing = False
            self.held_item_spr.scale = Vector2(0.7, 0.7)
            self.add_child(self.held_item_spr)
        self.held_item_spr.opacity = 1.0
        self.held_item_spr.frame_count_x = item_frame_count[item.id]
        self.held_item_spr.playing = False
        self.held_item_spr.transparent_color  = Color(0x07e0)
        self.held_item_spr.layer = 6

    def add_inv_item(self, item, data0):
        if(item_weights[item] + self.wt <= self.maxwt):
            self.inventory.append(Item(item, data0))
            self.wt += item_weights[item]
            return True
        else:
            return False

    def tick(self, dt):
        self.time += dt
        self.scale.x = 1.0
        if(self.held_item is not None):
            self.held_item_spr.texture = self.held_item.texture
            self.held_item_spr.position = Vector2(8, 0)
            if(self.look_ang < -math.pi/2 or self.look_ang > math.pi/2):
                self.scale.x = -1.0
                self.held_item_spr.position = Vector2(-8, 0)
            self.held_item_spr.rotation = self.look_ang

        sint = math.sin(4*self.time)
        self.scale.x *= (s_and_s_factor + ((1-s_and_s_factor)/2)*(1-sint))
        self.scale.y *= (s_and_s_factor + ((1-s_and_s_factor)/2)*sint)
