from engine_resources import TextureResource
from engine_math import Vector2

grass1 = TextureResource("GrassTile1_16bit.bmp")
grass2 = TextureResource("GrassTile2_16bit.bmp")
stone1 = TextureResource("Stone1Green_16bit.bmp")
stonefloor1 = TextureResource("Stone1GreenFloor_16bit.bmp")
stonecracked1 = TextureResource("Stone1GreenCracked_16bit.bmp")
water1 = TextureResource("Water1_16bit.bmp")
water2 = TextureResource("Water2_16bit.bmp")

trapdoor_sheet = TextureResource("trapdoor_sheet_16bit.bmp")
door_sheet = TextureResource("Door-Sheet_16bit.bmp")
chest_sheet = TextureResource("Chest1-Sheet_16bit.bmp")
spikes_sheet = TextureResource("Spikes-Sheet_16bit.bmp")
ladder_sheet = TextureResource("StoneLadder_16bit.bmp")

action_indicator = TextureResource("Indicator1_16bit.bmp") # Yellow indicator
attack_indicator = TextureResource("Indicator3_16bit.bmp") # Red indicator

grass_patch = TextureResource("GrassPatch1_16bit.bmp")

tile_ids = {
    "grass1": 0,
    "grass2": 1,
    "stone1": 2,
    "stonecracked1": 3,
    "water1": 4,
    "water2": 5,
    "door_sheet": 6,
    "stonefloor1": 7,
}
    
tile_textures = {
    tile_ids["grass1"]: grass1,
    tile_ids["grass2"]: grass2,
    tile_ids["stone1"]: stone1,
    tile_ids["stonecracked1"]: stonecracked1,
    tile_ids["water1"]: water1,
    tile_ids["water2"]: water2,
    tile_ids["door_sheet"]: door_sheet,
    tile_ids["stonefloor1"]: stonefloor1,
}

deco_ids = {
    "none": 0,
    "grass_patch": 1,
    "door_sheet": 2,
    "trapdoor_sheet": 3,
    "ladder_sheet": 4,
    "chest_sheet": 5,
}

deco_textures = {
    deco_ids["none"]: None,
    deco_ids["grass_patch"]: grass_patch,
    deco_ids["door_sheet"]: door_sheet,
    deco_ids["trapdoor_sheet"]: trapdoor_sheet,
    deco_ids["ladder_sheet"]: ladder_sheet,
    deco_ids["chest_sheet"]: chest_sheet,
}

deco_frame_count = {
    deco_ids["none"]: 0,
    deco_ids["grass_patch"]: 1,
    deco_ids["door_sheet"]: 3,
    deco_ids["trapdoor_sheet"]: 2,
    deco_ids["ladder_sheet"]: 1,
    deco_ids["chest_sheet"]: 2,
}

class Tilemap:
    def __init__(self, w=96, h=96):
        self.WIDTH = w
        self.HEIGHT = h
        self.tiles = bytearray(self.WIDTH*self.HEIGHT*3)
        self.border_tile = 2
        self.entryway = Vector2(1,1)
        self.monster_list = []
        self.spawn_list = [0]
        self.loot_list = [0]
    
    def get_tile_id(self, x, y):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return self.border_tile
        return self.tiles[int(y*self.WIDTH+x)*3]
    
    def get_tile_data0(self, x, y):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return 0
        return self.tiles[int(y*self.WIDTH+x)*3+1]
    
    def get_tile_data1(self, x, y):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return 1
        return self.tiles[int(y*self.WIDTH+x)*3+2]
    
    def set_tile_id(self, x, y, t):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return None
        self.tiles[int(y*self.WIDTH+x)*3] = t
        
    def set_tile_data0(self, x, y, d):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return None
        self.tiles[int(y*self.WIDTH+x)*3+1] = d
    
    def set_tile_data1(self, x, y, d):
        if((x < 0) or (x > self.WIDTH-1) or (y < 0) or (y > self.HEIGHT - 1)):
            return None
        self.tiles[int(y*self.WIDTH+x)*3+2] = d
        
    def set_tile_solid(self, x, y, solid):
        if(solid):
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) | (1))
        else:
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) & ~(1))
            
    def tile_solid(self, x, y):
        if((self.get_tile_data1(x, y) & 1) != 0):
            return True
        return False
            
    def set_deco_under(self, x, y, under):
        if(under):
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) | (1 << 1))
        else:
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) & ~(1 << 1))
            
    def set_deco_item(self, x, y, item):
        if(item):
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) | (1 << 2))
        else:
            self.set_tile_data1(x, y, self.get_tile_data1(x, y) & ~(1 << 2))