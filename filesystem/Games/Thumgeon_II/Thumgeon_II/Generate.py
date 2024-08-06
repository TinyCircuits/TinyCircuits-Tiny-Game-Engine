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

@micropython.native
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
        tilemap.tiles[(y*tilemap.WIDTH+x-1)*Tiles.TILE_DATA_BYTES] = adj

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
        tilemap.tiles[(y*tilemap.WIDTH+x+1)*Tiles.TILE_DATA_BYTES] = adj

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
        tilemap.tiles[((y-1)*tilemap.WIDTH+x)*Tiles.TILE_DATA_BYTES] = adj

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
        tilemap.tiles[((y+1)*tilemap.WIDTH+x)*Tiles.TILE_DATA_BYTES] = adj

overworld_loot_list = [
    Player.item_ids["apple"],
    Player.item_ids["clover"],
    Player.item_ids["dagger"],
]

dungeon_loot_list = [
    Player.item_ids["apple"],
    Player.item_ids["ham"],
    Player.item_ids["bomb"],
    # Player.item_ids["cat_bomb"],
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

MAX_WIDTH = 96
MAX_HEIGHT = 96

dungeons = {}

class Dungeon:
    def __init__(self, levels = 5, pos = Vector2(0, 0)):
        self.dungeon_levels = []
        for i in range(levels):
            self.dungeon_levels.append(Tiles.Tilemap(urandom.randrange(5, 9), urandom.randrange(5, 9)))
            shop = False
            if(urandom.randrange(0, 5) == 0):
                shop = True
            generate_empty_dungeon(self.dungeon_levels[i], True if i == 0 else False)
            generate_dungeon_level(self.dungeon_levels[i], True if i > 0 else False, True if i < levels-1 else False, False if i < levels-1 else True, shop)
        dungeons.update({pos.y*MAX_WIDTH+pos.x: self})

@micropython.native
def generate_tiles(tilemap):
    tilemap.loot_list = overworld_loot_list
    tilemap.spawn_list = [Monsters.monster_ids["slime"], Monsters.monster_ids["scorpion"], Monsters.monster_ids["chupacabra"]]
    fs = [generate_left, generate_right, generate_top, generate_bottom]
    for y in range(0, tilemap.HEIGHT if tilemap.HEIGHT < MAX_HEIGHT else MAX_HEIGHT):
        for x in range(0, tilemap.WIDTH if tilemap.WIDTH < MAX_WIDTH else MAX_WIDTH):
            if(int(tilemap.get_tile_id(x, y)) == 255):
                # Generate random tile
                tilemap.tiles[(y*tilemap.WIDTH+x)*Tiles.TILE_DATA_BYTES] = urandom.randrange(2)
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
    for i in range(12):
        # Generate 12 dungeon entrances
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
        dungeon = Dungeon(5, Vector2(dx, dy))


@micropython.native
def get_free_pos(tilemap, xmin, xmax, ymin, ymax):
    px = urandom.randrange(xmin, xmax)
    py = urandom.randrange(ymin, ymax)
    looking = True
    looped = False
    monster_occupied = True
    while(((tilemap.get_tile_data0(px, py) != 0) or (tilemap.tile_solid(px, py) == True) or monster_occupied) and looking):
        monster_occupied = False
        for m in tilemap.monster_list:
            if(m.position.x == px and m.position.y == py):
                monster_occupied = True
        px += 1
        if(px >= xmax):
            px = xmin
            py += 1
        if(py >= ymax and not looped):
            py = ymin
            looped = True
        else:
            looking = False
    return Vector2(px, py)


@micropython.native
def generate_empty_dungeon(tilemap, door = True):
    tilemap.spawn_list = [Monsters.monster_ids["skeleton"], Monsters.monster_ids["scorpion"], Monsters.monster_ids["ghost1"], Monsters.monster_ids["ghost2"], Monsters.monster_ids["slime"], Monsters.monster_ids["tempest"]]
    tilemap.loot_list = dungeon_loot_list
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

@micropython.native
def generate_item(tilemap, x, y):
    if(tilemap.get_tile_data0(x, y) == 0):
        item = urandom.choice(tilemap.loot_list)
        data0 = 0
        if(item == Player.item_ids["blue_book"]):
            data0 = Player.spell_ids[urandom.choice(Player.blue_spells)]
        elif(item == Player.item_ids["red_book"]):
            data0 = Player.spell_ids[urandom.choice(Player.red_spells)]
        else:
            # Random item quality
            data0 = urandom.randrange(5)
        tilemap.set_tile_data0(x, y, item)
        tilemap.set_tile_data1(x, y, 4)
        tilemap.set_tile_data2(x, y, data0)

@micropython.native
def generate_dungeon_monsters(tilemap, n):
    for i in range(n):
        pos = get_free_pos(tilemap, 2, tilemap.WIDTH-2, 2, tilemap.HEIGHT-2)
        m = Monsters.Monster()
        m.set_monster(urandom.choice(tilemap.spawn_list))
        m.position = pos
        tilemap.monster_list.append(m)

@micropython.native
def generate_dungeon_level(tilemap, ladder = True, trapdoor = True, rune = False, shop = True):
    if(trapdoor == True):
        pos = get_free_pos(tilemap, 1, tilemap.WIDTH-1, 1, tilemap.HEIGHT-1)
        tilemap.set_tile_data0(pos.x, pos.y, Tiles.deco_ids["trapdoor_sheet"])
        tilemap.set_deco_under(pos.x, pos.y, True)
    if(ladder == True):
        pos = get_free_pos(tilemap, 1, tilemap.WIDTH-1, 0, 1)
        tilemap.set_tile_data0(pos.x, pos.y, Tiles.deco_ids["ladder_sheet"])
        tilemap.set_deco_under(pos.x, pos.y, True)
        tilemap.entryway = Vector2(pos.x, 1)
    if(shop == True):
        pos = get_free_pos(tilemap, 1, tilemap.WIDTH-1, 1, tilemap.HEIGHT-1)
        tilemap.set_tile_data0(pos.x, pos.y, Tiles.deco_ids["shopkeep1"])
        tilemap.set_deco_under(pos.x, pos.y, True)

        item_count = urandom.randrange(5, 12)
        for i in range(item_count):
            item = urandom.choice(tilemap.loot_list)
            while(item == Player.item_ids["gold_coins"] or item == Player.item_ids["gold_bars"]):
                item = urandom.choice(tilemap.loot_list)
            data0 = 0
            if(item == Player.item_ids["blue_book"]):
                data0 = Player.spell_ids[urandom.choice(Player.blue_spells)]
            elif(item == Player.item_ids["red_book"]):
                data0 = Player.spell_ids[urandom.choice(Player.red_spells)]
            elif(item == Player.item_ids["bow"] or item == Player.item_ids["sword"] or item == Player.item_ids["dagger"]):
                # Random item quality
                data0 = urandom.randrange(len(Player.item_qualities))
            else:
                data0 = Player.item_qualities[""]
            tilemap.shopkeep_inv.append(Player.Item(item, data0))
    for i in range(3):
        pos = get_free_pos(tilemap, 2, tilemap.WIDTH-2, 2, tilemap.HEIGHT-2)
        generate_item(tilemap, pos.x, pos.y)
    if(urandom.random() < 1.0):
        pos = get_free_pos(tilemap, 2, tilemap.WIDTH-2, 2, tilemap.HEIGHT-2)
        tilemap.set_tile_data0(pos.x, pos.y, Tiles.deco_ids["chest_sheet"])
        tilemap.set_deco_under(pos.x, pos.y, True)
    if(rune == True):
        # Spawn a rune somewhere
        pos = get_free_pos(tilemap, 1, tilemap.WIDTH-1, 1, tilemap.HEIGHT-1)
        rune = Player.item_ids["rune1"] + urandom.randrange(8)
        looking = True
        print("Placing rune at "+str(pos.x) +", " + str(pos.y))
        tilemap.set_tile_data0(pos.x, pos.y, rune)
        tilemap.set_tile_data1(pos.x, pos.y, 4)
    generate_dungeon_monsters(tilemap, urandom.randrange(0, 2))
