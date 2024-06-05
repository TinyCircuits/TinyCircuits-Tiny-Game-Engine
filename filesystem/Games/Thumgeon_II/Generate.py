from engine_resources import NoiseResource
from engine_math import Vector2
import urandom
import utime
import Monsters

import Tiles
import Player

adjacency_likelihoods = {
    Tiles.tile_ids["grass1"]: {Tiles.tile_ids["grass1"]: 0.75, Tiles.tile_ids["grass2"]: 0.25, Tiles.tile_ids["stone1"]: 0.0, Tiles.tile_ids["stonecracked1"]: 0.0}, # Grass0
    Tiles.tile_ids["grass2"]: {Tiles.tile_ids["grass1"]: 0.25, Tiles.tile_ids["grass2"]: 0.75, Tiles.tile_ids["stone1"]: 0.0, Tiles.tile_ids["stonecracked1"]: 0.0}, # Grass1
    Tiles.tile_ids["stone1"]: {Tiles.tile_ids["grass1"]: 0.125, Tiles.tile_ids["grass2"]: 0.125, Tiles.tile_ids["stone1"]: 0.5, Tiles.tile_ids["stonecracked1"]: 0.25},		  # Stone
    Tiles.tile_ids["stonecracked1"]: {Tiles.tile_ids["grass1"]: 0.125, Tiles.tile_ids["grass2"]: 0.125, Tiles.tile_ids["stone1"]: 0.5, Tiles.tile_ids["stonecracked1"]: 0.25},		  # Cracked stone
}

def shuffle(arr):
    for i in range(len(arr)-1, 0 -1):
        j = urandom.randrange(i+1)
        arr[i], arr[j] = arr[j], arr[i]

@micropython.native
def generate_left(tilemap, x, y):
    global adjacency_likelihoods
    id = tilemap.get_tile_id(x, y)
    if((x > 0) and tilemap.get_tile_id(x-1, y) == 255): # Generate left tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        tilemap.tiles[(y*tilemap.WIDTH+x-1)*3] = adj
        
@micropython.native
def generate_right(tilemap, x, y):
    global adjacency_likelihoods
    id = tilemap.get_tile_id(x, y)
    if((x < tilemap.WIDTH) and tilemap.get_tile_id(x+1, y) == 255): # Generate right tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        tilemap.tiles[(y*tilemap.WIDTH+x+1)*3] = adj

@micropython.native
def generate_top(tilemap, x, y):
    global adjacency_likelihoods
    id = tilemap. get_tile_id(x, y)
    if((y > 0) and tilemap.get_tile_id(x, y-1) == 255): # Generate top tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        tilemap.tiles[((y-1)*tilemap.WIDTH+x)*3] = adj

@micropython.native
def generate_bottom(tilemap, x, y):
    global adjacency_likelihoods
    id = tilemap.get_tile_id(x, y)
    if((y < tilemap.HEIGHT) and tilemap.get_tile_id(x, y+1) == 255): # Generate bottom tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        tilemap.tiles[((y+1)*tilemap.WIDTH+x)*3] = adj


@micropython.native
def generate_tiles(tilemap):
    tilemap.loot_list = overworld_loot_list
    tilemap.spawn_list = [Monsters.monster_ids["slime"], Monsters.monster_ids["scorpion"], Monsters.monster_ids["chupacabra"]]
    fs = [generate_left, generate_right, generate_top, generate_bottom]
    for y in range(0, tilemap.HEIGHT):
        for x in range(0, tilemap.WIDTH):
            if(int(tilemap.get_tile_id(x, y)) == 255):
                # Generate random tile
                tilemap.tiles[(y*tilemap.WIDTH+x)*3] = urandom.randrange(4)
            shuffle(fs)
            # Each tile only generates two neighbors
            for i in range(3):
                fs[i](tilemap, x, y)


perlin = NoiseResource()
perlin.seed = utime.ticks_us()

@micropython.native
def generate_water(tilemap):
    for y in range(0, tilemap.HEIGHT):
        for x in range(0, tilemap.WIDTH):
            if(perlin.noise_2d(x*10, y*10) < 0.001):
                tilemap.set_tile_id(x, y, Tiles.tile_ids["water1"])
                #tilemap.set_tile_data1(x, y, 255)
                tilemap.set_tile_solid(x, y, True)

@micropython.native
def generate_deco(tilemap):
    for y in range(0, tilemap.HEIGHT):
        for x in range(0, tilemap.WIDTH):
            tile_id = tilemap.get_tile_id(x, y)
            if(tile_id == Tiles.tile_ids["grass1"] or tile_id == Tiles.tile_ids["grass2"]):
                # 10% chance of having grass decoration
                if(urandom.random() < 0.1):
                    tilemap.set_tile_data0(x, y, Tiles.deco_ids["grass_patch"])
                    tilemap.set_deco_under(x, y, False)
            '''
            elif(tile_id == Tiles.tile_ids["stone1"]):
                if(urandom.random() < 0.1):
                    tilemap.set_tile_data0(x, y, Tiles.deco_ids["door_sheet"])
                    tilemap.set_deco_under(x, y, True)
            '''
    for i in range(12):
        # Generate dungeon entrances
        dx = urandom.randrange(tilemap.WIDTH)
        dy = urandom.randrange(tilemap.HEIGHT)
        looking = True
        while(tilemap.get_tile_data0(dx, dy) != 0):
            looking = False
            for y in range(dy-1, dy+2):
                for x in range(dx-1, dx+2):
                    if(tilemap.get_tile_data0(dx, dy) != 0):
                        looking = True
            if(looking == True):
                dx += 1
                if(dx >= tilemap.WIDTH):
                    dx = 0
                    dy += 1
                if(dy >= tilemap.HEIGHT):
                    dy = 0
        for y in range(dy-2, dy+1):
            for x in range(dx-1, dx+2):
                tilemap.set_tile_id(x, y, Tiles.tile_ids["stone1"])
                tilemap.set_tile_data0(x, y, 0)
                tilemap.set_tile_solid(x, y, True)
        tilemap.set_tile_id(dx, dy, Tiles.tile_ids["stone1"])
        tilemap.set_tile_data0(dx, dy, Tiles.deco_ids["door_sheet"])
        tilemap.set_deco_under(dx, dy, True)
        tilemap.set_tile_solid(dx, dy, True)
        
        

@micropython.native
def generate_empty_dungeon(tilemap, door = True):
    tilemap.spawn_list = [Monsters.monster_ids["skeleton"], Monsters.monster_ids["scorpion"], Monsters.monster_ids["ghost1"], Monsters.monster_ids["ghost2"], Monsters.monster_ids["slime"], Monsters.monster_ids["tempest"]]
    for y in range(1, tilemap.HEIGHT-1):
        for x in range(1, tilemap.WIDTH-1):
            tilemap.set_tile_id(x, y, Tiles.tile_ids["stonefloor1"])
    for y in range(0, tilemap.HEIGHT):
        tilemap.set_tile_id(0, y, Tiles.tile_ids["stone1"])
        tilemap.set_tile_data1(0, y, 1)
        tilemap.set_tile_id(tilemap.WIDTH-1, y, Tiles.tile_ids["stone1"])
        tilemap.set_tile_data1(tilemap.WIDTH-1, y, 1)
    for x in range(1, tilemap.WIDTH-1):
        tilemap.set_tile_id(x, 0, Tiles.tile_ids["stone1"])
        tilemap.set_tile_data1(x, 0, 1)
        tilemap.set_tile_id(x, tilemap.HEIGHT-1, Tiles.tile_ids["stone1"])
        tilemap.set_tile_data1(x, tilemap.HEIGHT-1, 1)
    if(door == True):
        exit_side = urandom.randrange(4)
        if exit_side == 0:
            door_offset = urandom.randrange(1, tilemap.WIDTH-1)
            tilemap.set_tile_data0(door_offset, 0, Tiles.deco_ids["door_sheet"])
            tilemap.set_deco_under(door_offset, 0, True)
            tilemap.entryway = Vector2(door_offset, 1)
        elif exit_side == 1:
            door_offset = urandom.randrange(1, tilemap.WIDTH-1)
            tilemap.set_tile_data0(door_offset, tilemap.HEIGHT-1, Tiles.deco_ids["door_sheet"])
            tilemap.set_deco_under(door_offset, tilemap.HEIGHT-1, True)
            tilemap.entryway = Vector2(door_offset, tilemap.HEIGHT-2)
        elif exit_side == 2:
            door_offset = urandom.randrange(1, tilemap.HEIGHT-1)
            tilemap.set_tile_data0(0, door_offset, Tiles.deco_ids["door_sheet"])
            tilemap.set_deco_under(0, door_offset, True)
            tilemap.entryway = Vector2(1, door_offset)
        elif exit_side == 3:
            door_offset = urandom.randrange(1, tilemap.HEIGHT-1)
            tilemap.set_tile_data0(tilemap.WIDTH-1, door_offset, Tiles.deco_ids["door_sheet"])
            tilemap.entryway = Vector2(tilemap.WIDTH-2, door_offset)
            
overworld_loot_list = [
    Player.item_ids["apple"],
    Player.item_ids["clover"],
    Player.item_ids["dagger"],
]

dungeon_loot_list = [
    Player.item_ids["apple"],
    Player.item_ids["ham"],
    Player.item_ids["bomb"],
    Player.item_ids["cat_bomb"],
    Player.item_ids["bow"],
    Player.item_ids["red_book"],
    Player.item_ids["blue_book"],
    Player.item_ids["bow"],
    Player.item_ids["dagger"],
    Player.item_ids["sword"],
    Player.item_ids["gold_coins"],
    Player.item_ids["gold_bars"],
    Player.item_ids["blue_pot"],
    Player.item_ids["red_pot"],
]

def generate_dungeon_level(tilemap, ladder = True, trapdoor = True, rune = False):
    if(trapdoor == True):
        exit_x = urandom.randrange(1, tilemap.WIDTH-1)
        exit_y = urandom.randrange(1, tilemap.HEIGHT-1)
        tilemap.set_tile_data0(exit_x, exit_y, Tiles.deco_ids["trapdoor_sheet"])
        tilemap.set_deco_under(exit_x, exit_y, True)
    if(ladder == True):
        ladder_x = urandom.randrange(1, tilemap.WIDTH-1)
        tilemap.set_tile_data0(ladder_x, 0, Tiles.deco_ids["ladder_sheet"])
        tilemap.set_deco_under(ladder_x, 0, True)
        tilemap.entryway = Vector2(ladder_x, 1)
    for i in range(3):
        item_x = urandom.randrange(1, tilemap.WIDTH-1)
        item_y = urandom.randrange(1, tilemap.HEIGHT-1)
        if(tilemap.get_tile_data0(item_x, item_y) == 0):
            item = urandom.choice(tilemap.loot_list)
            tilemap.set_tile_data0(item_x, item_y, item)
            tilemap.set_tile_data1(item_x, item_y, 4)
        #tilemap.set_deco_under(exit_x, exit_y, True)
    if(urandom.random() < 1.0):
        # Generate chest
        chest_x = urandom.randrange(1, tilemap.WIDTH-1)
        chest_y = urandom.randrange(1, tilemap.HEIGHT-1)
        looking = True
        while(tilemap.get_tile_data0(chest_x, chest_y) != 0):
            chest_x += 1
            if(chest_x >= tilemap.WIDTH):
                chest_x = 0
                chest_y += 1
            if(chest_y >= tilemap.HEIGHT):
                chest_y = 0
        tilemap.set_tile_data0(chest_x, chest_y, Tiles.deco_ids["chest_sheet"])
        tilemap.set_deco_under(chest_x, chest_y, True)
    
