import engine_main
import engine
import engine_io
from tilemapper import TTileType, TilingRenderer, tile_rules
from engine_nodes import CameraNode
from engine_resources import TextureResource
from engine_math import Vector3

import random, time
from math import sin, cos, radians, sqrt, log, pi
from array import array

def generate_cellular_automata(width, height, seed, fill_percent, edges_are_walls):
    random.seed(seed)
    
    # Initialize the map with array type
    map = array('b', [0] * (width * height))
    
    for x in range(width):
        for y in range(height):
            if edges_are_walls and (x == 0 or x == width - 1 or y == 0 or y == height - 1):
                map[x * height + y] = 1
            else:
                map[x * height + y] = 1 if random.randint(0, 100) < fill_percent else 0
                
    return map

def get_moore_surrounding_tiles(map, x, y, width, height, edges_are_walls):
    tile_count = 0
    
    for neighbour_x in range(x - 1, x + 2):
        for neighbour_y in range(y - 1, y + 2):
            if 0 <= neighbour_x < width and 0 <= neighbour_y < height:
                if neighbour_x != x or neighbour_y != y:
                    tile_count += map[neighbour_x * height + neighbour_y]
                    
    return tile_count

def smooth_moore_cellular_automata(map, width, height, edges_are_walls, smooth_count):
    for _ in range(smooth_count):
        new_map = array('b', [0] * (width * height))
        for x in range(width):
            for y in range(height):
                surrounding_tiles = get_moore_surrounding_tiles(map, x, y, width, height, edges_are_walls)
                
                if edges_are_walls and (x == 0 or x == width - 1 or y == 0 or y == height - 1):
                    new_map[x * height + y] = 1
                elif surrounding_tiles > 4:
                    new_map[x * height + y] = 1
                elif surrounding_tiles < 4:
                    new_map[x * height + y] = 0
                else:
                    new_map[x * height + y] = map[x * height + y]
        map = new_map
    
    return map

def draw_line_on_map(map, width, height, start, end, line_width):
    w2 = (line_width / 2) ** 2
    for x in range(width):
        for y in range(height):
            if distance_from_point_to_segment(start, end, (x, y)) <= w2:
                map[x * height + y] = 1
    return map

def draw_circle_on_map(map, width, height, center, circle_width):
    for x in range(width):
        for y in range(height):
            if distance(center, (x, y)) <= circle_width / 2:
                map[x * height + y] = 1
    return map

def draw_stroked_line_on_map(map, width, height, start, end, line_width, core_width):
    w2 = (line_width / 2) ** 2
    c2 = (core_width / 2) ** 2
    for x in range(width):
        for y in range(height):
            dist = distance_from_point_to_segment(start, end, (x, y))
            if dist <= w2 and dist > c2:
                map[x * height + y] = 1
    return map

def distance(p1, p2):
    return sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)

def distance_from_point_to_segment(a, b, p):
    n = (b[0] - a[0], b[1] - a[1])
    pa = (a[0] - p[0], a[1] - p[1])
    c = n[0] * pa[0] + n[1] * pa[1]
    if c > 0.0:
        return pa[0] ** 2 + pa[1] ** 2
    bp = (p[0] - b[0], p[1] - b[1])
    if n[0] * bp[0] + n[1] * bp[1] > 0.0:
        return bp[0] ** 2 + bp[1] ** 2
    e = (pa[0] - n[0] * (c / (n[0] ** 2 + n[1] ** 2)), pa[1] - n[1] * (c / (n[0] ** 2 + n[1] ** 2)))
    return e[0] ** 2 + e[1] ** 2

def generate_random_walk_river(map, width, height, start, length):
    x, y = start
    directions = [(1, 0), (0, 1), (-1, 0), (0, -1)]
    
    for _ in range(length):
        map[x * height + y] = 1
        direction = random.choice(directions)
        x = (x + direction[0]) % width
        y = (y + direction[1]) % height
        if map[x * height + y] == 1:
            break
    return map

def gaussian_random(mean, stddev):
    u1 = random.random()
    u2 = random.random()
    z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * pi * u2)
    return z0 * stddev + mean

def generate_golf_hole(global_width, global_height, seed):
    random.seed(seed)

    world_scale_factor = 6

    # Define the parameters for the golf hole generation
    hole_length = int(gaussian_random(20, 5) / world_scale_factor)
    fairway_width = random.randint(2, 4)
    green_width = fairway_width - 1
    density = random.randint(66, 68)
    smoothness = random.randint(5, 30)

    # Generate the rough background
    rough_map = generate_cellular_automata(global_width, global_height, seed, 70, True)

    # Generate the fairway
    fairway_map = generate_cellular_automata(global_width, global_height, seed, density, True)
    fairway_map = smooth_moore_cellular_automata(fairway_map, global_width, global_height, False, smoothness)

    # Generate the green
    green_map = generate_cellular_automata(global_width, global_height, seed, 65, True)
    green_map = smooth_moore_cellular_automata(green_map, global_width, global_height, False, 10)

    # Draw fairway and green shapes
    hole_shape = [(global_width // 2, 0)]
    launch_point = hole_shape[0]
    distance_so_far = 0

    while distance_so_far < hole_length:
        segment_distance = gaussian_random(20, 3) / world_scale_factor
        if distance_so_far + segment_distance > hole_length:
            segment_distance = hole_length - distance_so_far

        degree = gaussian_random(0, 15)
        radian = radians(degree + 90)
        x = int(launch_point[0] + segment_distance * cos(radian))
        y = int(launch_point[1] + segment_distance * sin(radian))

        hole_shape.append((x, y))
        launch_point = (x, y)
        distance_so_far += segment_distance

    # Draw fairway and green shapes on the map
    for i in range(len(hole_shape) - 1):
        fairway_map = draw_line_on_map(fairway_map, global_width, global_height, hole_shape[i], hole_shape[i+1], fairway_width)

    green_center = hole_shape[-1]
    green_map = draw_circle_on_map(green_map, global_width, global_height, green_center, green_width)

    # Generate water hazards
    water_map = generate_cellular_automata(global_width, global_height, seed, 40, True)
    for _ in range(random.randint(1, 3)):
        start = (random.randint(0, global_width - 1), random.randint(0, global_height - 1))
        length = random.randint(10, 50)
        water_map = generate_random_walk_river(water_map, global_width, global_height, start, length)
    water_map = smooth_moore_cellular_automata(water_map, global_width, global_height, False, 3)

    # Generate bunkers
    bunker_map = generate_cellular_automata(global_width, global_height, seed, 30, True)
    for _ in range(random.randint(1, 3)):
        center = (random.randint(0, global_width - 1), random.randint(0, global_height - 1))
        width = random.randint(3, 7)
        bunker_map = draw_circle_on_map(bunker_map, global_width, global_height, center, width)
    bunker_map = smooth_moore_cellular_automata(bunker_map, global_width, global_height, False, 3)

    return rough_map, fairway_map, green_map, water_map, bunker_map

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
sand_tile = TTileType('sand', sand_texture, frame_count_x=3, frame_count_y=6)
green_tile = TTileType('green', green_texture, frame_count_x=3, frame_count_y=6)
trees_tile = TTileType('dirt', trees_texture, frame_count_x=3, frame_count_y=6)

# Define map dimensions and parameters
global_width = 20
global_height = 40
seed = time.ticks_ms()

# Generate a random golf hole
rough_map, fairway_map, green_map, water_map, bunker_map = generate_golf_hole(global_width, global_height, seed)

# Initialize renderers for each map
#renderer_rough = TilingRenderer(rough_map, grass_tile, tile_rules, global_width, global_height)
#renderer_rough.set_layer(1)

renderer_fairway = TilingRenderer(fairway_map, grass_tile, tile_rules, global_width, global_height)
renderer_fairway.set_layer(2)

#renderer_green = TilingRenderer(green_map, green_tile, tile_rules, global_width, global_height)
#renderer_green.set_layer(3)

renderer_water = TilingRenderer(water_map, water_tile, tile_rules, global_width, global_height)
renderer_water.set_layer(4)

renderer_bunker = TilingRenderer(bunker_map, sand_tile, tile_rules, global_width, global_height)
renderer_bunker.set_layer(5)

class MovingCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.zoom_levels = [0.125, 0.25, 0.5, 1, 2, 3, 4, 5]
        self.zoom_index = 3  # Start with zoom level 1 (index 3)
        self.zoom = self.zoom_levels[self.zoom_index]

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

        if engine_io.check_just_pressed(engine_io.B):
            self.zoom_index = max(0, self.zoom_index - 1)
            self.zoom = self.zoom_levels[self.zoom_index]
        elif engine_io.check_just_pressed(engine_io.A):
            self.zoom_index = min(len(self.zoom_levels) - 1, self.zoom_index + 1)
            self.zoom = self.zoom_levels[self.zoom_index]

camera = MovingCamera()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)

engine.start()
