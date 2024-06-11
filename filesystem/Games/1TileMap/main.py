import engine_main
import engine
from tilemapper import TTileType, TileRules, TileRule, TilingRenderer
from engine_nodes import CameraNode
from engine_resources import TextureResource
from engine_math import Vector3

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

grass_texture = TextureResource("grass.bmp")
water_texture = TextureResource("water.bmp")

# Define tile types
grass_tile = TTileType('grass', grass_texture, frame_count_x=3, frame_count_y=6)
water_tile = TTileType('water', water_texture, frame_count_x=3, frame_count_y=6)

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

# Tile maps for testing
tile_map_grass = [
    [0, 1, 1, 0, 1, 1],
    [1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1],
    [1, 1, 1, 0, 1, 1]
]

tile_map_water = [
    [0, 0, 0, 0, 0, 0],
    [0, 1, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0],
    [0, 0, 0, 1, 1, 0],
    [0, 0, 0, 1, 1, 0],
    [0, 0, 0, 0, 0, 0],
]

# Initialize renderers
renderer_grass = TilingRenderer(tile_map_grass, grass_tile, tile_rules)
renderer_grass.set_layer(0)

renderer_water = TilingRenderer(tile_map_water, water_tile, tile_rules)
renderer_water.set_layer(1)

camera = CameraNode()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)

engine.start()
