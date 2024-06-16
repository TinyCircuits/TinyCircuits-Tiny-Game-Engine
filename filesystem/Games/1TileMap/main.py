import engine_main
import engine
import engine_io
from tilemapper import TTileType, TileRules, TileRule, TilingRenderer
from engine_nodes import CameraNode
from engine_resources import TextureResource
from engine_math import Vector3


import random, time

def generate_cellular_automata(width, height, seed, fill_percent, edges_are_walls):
    random.seed(seed)
    
    # Initialize the map
    map = [[0 for _ in range(height)] for _ in range(width)]
    
    for x in range(width):
        for y in range(height):
            if edges_are_walls and (x == 0 or x == width - 1 or y == 0 or y == height - 1):
                map[x][y] = 1
            else:
                map[x][y] = 1 if random.randint(0, 100) < fill_percent else 0
                
    return map

def get_moore_surrounding_tiles(map, x, y, edges_are_walls):
    tile_count = 0
    width = len(map)
    height = len(map[0])
    
    for neighbour_x in range(x - 1, x + 2):
        for neighbour_y in range(y - 1, y + 2):
            if 0 <= neighbour_x < width and 0 <= neighbour_y < height:
                if neighbour_x != x or neighbour_y != y:
                    tile_count += map[neighbour_x][neighbour_y]
                    
    return tile_count

def smooth_moore_cellular_automata(map, edges_are_walls, smooth_count):
    width = len(map)
    height = len(map[0])
    
    for _ in range(smooth_count):
        new_map = [[0 for _ in range(height)] for _ in range(width)]
        for x in range(width):
            for y in range(height):
                surrounding_tiles = get_moore_surrounding_tiles(map, x, y, edges_are_walls)
                
                if edges_are_walls and (x == 0 or x == width - 1 or y == 0 or y == height - 1):
                    new_map[x][y] = 1
                elif surrounding_tiles > 4:
                    new_map[x][y] = 1
                elif surrounding_tiles < 4:
                    new_map[x][y] = 0
                else:
                    new_map[x][y] = map[x][y]
        map = new_map
    
    return map

def fade(t):
    return t * t * t * (t * (t * 6 - 15) + 10)

def lerp(a, b, t):
    return a + t * (b - a)

def grad(hash, x, y):
    h = hash & 7
    u = x if h < 4 else y
    v = y if h < 4 else x
    return (u if h & 1 == 0 else -u) + (v if h & 2 == 0 else -v)

def perlin_noise(x, y, perm):
    xi = int(x) & 255
    yi = int(y) & 255

    xf = x - int(x)
    yf = y - int(y)

    u = fade(xf)
    v = fade(yf)

    aa = perm[perm[xi] + yi]
    ab = perm[perm[xi] + yi + 1]
    ba = perm[perm[xi + 1] + yi]
    bb = perm[perm[xi + 1] + yi + 1]

    x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u)
    x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u)

    return (lerp(x1, x2, v) + 1) / 2  # Normalize to [0, 1]

# Permutation table for Perlin noise
perm = [151,160,137,91,90,15,
        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
        8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,
        32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,
        166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,
        143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,
        188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,
        255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
        2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,
        232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,
        50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,
        61,156,180]

# Duplicate the permutation list to avoid overflow
perm = perm * 2

def perlin_noise_map(map, modifier, edges_are_walls):
    for x in range(len(map)):
        for y in range(len(map[0])):
            if edges_are_walls and (x == 0 or y == 0 or x == len(map) - 1 or y == len(map[0]) - 1):
                map[x][y] = 1  # Keep the edges as walls
            else:
                # Generate a new point using Perlin noise, then round it to a value of either 0 or 1
                new_point = round(perlin_noise(x * modifier, y * modifier, perm))
                map[x][y] = new_point
    return map

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

grass_texture = TextureResource("grass.bmp")
water_texture = TextureResource("water.bmp")
sand_texture = TextureResource("sand.bmp")
green_texture = TextureResource("green.bmp")
trees_texture = TextureResource("dirt.bmp")

# Define tile types
grass_tile = TTileType('grass', grass_texture, frame_count_x=3, frame_count_y=6)
water_tile = TTileType('water', water_texture, frame_count_x=3, frame_count_y=6)
sand_tile = TTileType('water', sand_texture, frame_count_x=3, frame_count_y=6)
green_tile = TTileType('green', green_texture, frame_count_x=3, frame_count_y=6)
trees_tile = TTileType('dirt', trees_texture, frame_count_x=3, frame_count_y=6)

tile_rules = TileRules()
# Update rules to work with 3x3 patterns (0: empty, 1: present)

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    [1, 1, 1]
], [(1, 3, 5), (0, 5, 1), (1, 5, 1), (2, 5, 1)]))  # Example of weighted choices

tile_rules.add_rule(TileRule([
    [0, 0, 0],
    [0, 1, 0],
    [0, 0, 0]
], (0, 0)))

tile_rules.add_rule(TileRule([
    [1, 1, 0],
    [1, 1, 1],
    [0, 1, 1]
], (0, 1)))

tile_rules.add_rule(TileRule([
    ['*', 1, 1],
    [1, 1, 1],
    [1, 1, 1]
], (2, 1)))

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    ['*', 1, 1]
], (2, 0)))

tile_rules.add_rule(TileRule([
    [1, 1, '*'],
    [1, 1, 1],
    [1, 1, 1]
], (1, 1)))

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    [1, 1, '*']
], (1, 0)))


tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    [1, 1, 1],
    [1, 1, 1]
], (1, 2)))

tile_rules.add_rule(TileRule([
    ['*', 1, 1],
    ['*', 1, 1],
    ['*', 1, 1]
], (0, 3)))

tile_rules.add_rule(TileRule([
    [1, 1, '*'],
    [1, 1, '*'],
    [1, 1, '*']
], (2, 3)))

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    ['*', '*', '*']
], (1, 4)))

tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    [1, 1, '*'],
    [1, 1, '*']
], (2, 2)))

tile_rules.add_rule(TileRule([
    [1, 1, '*'],
    [1, 1, '*'],
    ['*', '*', '*']
], (2, 4)))

tile_rules.add_rule(TileRule([
    ['*', 1, 1],
    ['*', 1, 1],
    ['*', '*', '*']
], (0, 4)))

tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    ['*', 1, 1],
    ['*', 1, 1]
], (0, 2)))

# Define map dimensions and parameters
width = 20
height = 20
seed = time.ticks_ms()
fill_percent = 80
edges_are_walls = False
smooth_count = 5
modifier = 0.1

# Generate the initial cellular automata map
cellular_map_grass = generate_cellular_automata(width, height, seed, fill_percent, edges_are_walls)

# Apply Perlin noise
#perlin_map_grass = perlin_noise_map(cellular_map_grass, modifier, edges_are_walls)

# Smooth the map using Moore neighborhood rules
tile_map_grass = smooth_moore_cellular_automata(cellular_map_grass, edges_are_walls, smooth_count)

seed = time.ticks_ms()
fill_percent = 45

cellular_map_water = generate_cellular_automata(width, height, seed, fill_percent, edges_are_walls)

# Apply Perlin noise
#perlin_map_water = perlin_noise_map(cellular_map_water, modifier, edges_are_walls)

# Smooth the map using Moore neighborhood rules
tile_map_water = smooth_moore_cellular_automata(cellular_map_water, edges_are_walls, smooth_count)



import struct
import array

def load_combined_layers(filename):
    with open(filename, 'rb') as f:
        # Read image dimensions
        dimensions = f.read(8)
        width, height = struct.unpack('II', dimensions)
        
        # Calculate the size of a single layer
        layer_size = width * height
        
        # Read the rest of the file
        data = f.read()
        
    # Convert the byte array into a list of integers
    combined_data = array.array('B', data)
    
    # Separate the combined data into individual layers
    layers = {}
    layers['water'] = combined_data[0:layer_size]
    layers['sand'] = combined_data[layer_size:2*layer_size]
    layers['grass'] = combined_data[2*layer_size:3*layer_size]
    layers['green'] = combined_data[3*layer_size:4*layer_size]
    layers['trees'] = combined_data[4*layer_size:5*layer_size]
    
    # Convert each layer into a 2D array
    for key in layers:
        layer = layers[key]
        layer_2d = []
        for y in range(height):
            row = []
            for x in range(width):
                row.append(layer[y * width + x])
            layer_2d.append(row)
        layers[key] = layer_2d
    
    return layers

# Example usage
layers = load_combined_layers('combined_layers_with_dimensions.bin')
water_layer = layers['water']
sand_layer = layers['sand']
grass_layer = layers['grass']
green_layer = layers['green']
trees_layer = layers['trees']


# Now water_layer, sand_layer, green_layer, trees_layer, and grass_layer contain the 2D arrays



start=16
end=32

# Initialize renderers
renderer_grass = TilingRenderer([row[start:end] for row in grass_layer[start:end]], grass_tile, tile_rules)
renderer_grass.set_layer(1)

renderer_sand = TilingRenderer([row[start:end] for row in sand_layer[start:end]], sand_tile, tile_rules)
renderer_sand.set_layer(5)

renderer_water = TilingRenderer([row[start:end] for row in water_layer[start:end]], water_tile, tile_rules)
renderer_water.set_layer(4)

renderer_green = TilingRenderer([row[start:end] for row in green_layer[start:end]], green_tile, tile_rules)
renderer_green.set_layer(3)

renderer_trees = TilingRenderer([row[start:end] for row in trees_layer[start:end]], trees_tile, tile_rules)
renderer_trees.set_layer(2)


class MovingCamera(CameraNode):
    def __init__(self):
        super().__init__(self)

    def tick(self, dt):
        # NOTE: Game coordinate system is -y = to top of screen, +y = to bottom of screen
        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.position.y -= 1
        elif engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.position.y += 1
        
        # X axis works how it should
        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.position.x -= 1
        elif engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.position.x += 1

        if engine_io.check_pressed(engine_io.A):
            self.zoom -= 0.01
        elif engine_io.check_pressed(engine_io.B):
            self.zoom += .01

camera = MovingCamera()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)

engine.start()
