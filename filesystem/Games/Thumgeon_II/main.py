import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource, NoiseResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN
import math
import os
import gc
import urandom
import utime

print(os.getcwd())

grass1 = TextureResource("GrassTile1_16bit.bmp")
grass2 = TextureResource("GrassTile2_16bit.bmp")
stone1 = TextureResource("Stone1Green_16bit.bmp")
stonecracked1 = TextureResource("Stone1GreenCracked_16bit.bmp")
water1 = TextureResource("Water1_16bit.bmp")
water2 = TextureResource("Water2_16bit.bmp")
player_texture = TextureResource("PlayerTile_16bit.bmp")
door_sheet = TextureResource("Door-Sheet_16bit.bmp")
spikes_sheet = TextureResource("Spikes-Sheet_16bit.bmp")
action_indicator = TextureResource("Indicator1_16bit.bmp") # Yellow indicator

grass_patch = TextureResource("GrassPatch1_16bit.bmp")

perlin = NoiseResource()

engine_physics.set_gravity(0, 0.0)
#engine.set_fps_limit(120)
engine.disable_fps_limit()


class DrawTile(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = grass1
        self.frame_count_x = 1
        self.width = 32
        self.height = 32
        self.id = 0
        self.set_layer(1)
        #self.tween = Tween()
        
        self.deco = []
    
    @micropython.native
    def add_deco(self, deco):
        if deco is not None:
            deco_sprite = Sprite2DNode()
            deco_sprite.texture = deco
            deco_sprite.set_layer(2)
            deco_sprite.transparent_color = Color(0x07e0)
            #deco_sprite.opacity = 0.6
            self.add_child(deco_sprite)
            self.deco.append(deco_sprite)
            
    @micropython.native
    def reset_deco(self):
        '''
        for d in self.deco:
            self.remove_child(d)
            d.destroy()
        self.deco.clear()
        pass
        '''
        self.deco.clear()
        self.destroy_children()
        
    def collision(self, contact):
        pass
    
adjacency_likelihoods = {
    0: (0.75, 0.125, 7*0.125/8, 0.125/8), # Grass0
    1: (0.125, 0.75, 7*0.125/8, 0.125/8), # Grass1
    2: (0.125, 0.125, 0.5, 0.25),		  # Stone
    3: (0.125, 0.125, 0.5, 0.25),		  # Cracked stone
}

renderer_tiles = [None] * 6 * 6
for x in range(0, 6):
    for y in range(0, 6):
        renderer_tiles[y*6+x] = DrawTile()
        renderer_tiles[y*6+x].position = Vector2(32*(x), 32*(y))

WORLD_WIDTH = const(96)
WORLD_HEIGHT = const(96)

world_tiles = bytearray(WORLD_WIDTH*WORLD_HEIGHT*3)

@micropython.native
def get_tile_id(x, y):
    global world_tiles
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3]

@micropython.native
def get_tile_data0(x, y):
    global world_tiles
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3+1]

@micropython.native
def get_tile_data1(x, y):
    global world_tiles
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3+2]

@micropython.native
def tile_id_to_sprite(tile_id):
    global grass1
    global grass2
    if(tile_id == 0):
        return grass1
    elif(tile_id == 1):
        return grass2
    elif(tile_id == 2):
        return stone1
    elif(tile_id == 3):
        return stonecracked1
    elif(tile_id == 4):
        return water1
    elif(tile_id == 5):
        return water2
    elif(tile_id == 6):
        return door_sheet
    else:
        return None
    
@micropython.native
def tile_data_to_deco(tile_data):
    global grass1
    global grass2
    if(tile_data == 0):
        return None
    elif(tile_data == 1):
        return grass_patch
    else:
        return None
        
cam = CameraNode()
camera_tween = Tween()
camera_offset = Vector2(64, 64)

class PlayerSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = player_texture
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.set_layer(7)
        self.opacity = 1.0
        self.transparent_color = Color(0x07e0)
        self.width = 32
        self.height = 32
        cam.add_child(self)

player = PlayerSprite()

class CursorSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = action_indicator
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.set_layer(6)
        self.opacity = 0.0
        self.transparent_color = Color(0x07e0)
        self.width = 32
        self.height = 32
        self.tween = Tween()
        cam.add_child(self)
        
cursor = CursorSprite()
cursor_pos = Vector2(0, 0)

renderer_x = 0
renderer_y = 0

mode_move = 0
mode_action = 1

control_mode = mode_move

def reset_camera_right(self):
    global renderer_x
    renderer_x -= 1
    #load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_left(self):
    global renderer_x
    renderer_x += 1
    load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_down(self):
    global renderer_y
    renderer_y += 1
    load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_up(self):
    global renderer_y
    renderer_y -= 1
    #load_renderer_tiles(renderer_x, renderer_y)

@micropython.native
def load_renderer_tiles(cx, cy):
    cam.position = Vector3(64,64,1)
    for y in range(0, 6):
        for x in range(0, 6):
            renderer_tiles[y*6+x].reset_deco()
            renderer_tiles[y*6+x].id = get_tile_id(cx+x, cy+y)
            renderer_tiles[y*6+x].texture = tile_id_to_sprite(renderer_tiles[y*6+x].id)
            renderer_tiles[y*6+x].set_layer(1)
            renderer_tiles[y*6+x].add_deco(tile_data_to_deco(get_tile_data0(cx+x, cy+y)))
    
    
tween_snap = 80 # Controls camera tween speed
indicator_snap = 50 # Controls indicator tween speed

camera_z = 0.0

@micropython.native
def generate_left(x, y):
    global world_tiles
    global adjacency_likelihoods
    id = get_tile_id(x, y)
    if((x > 0) and get_tile_id(x-1, y) == 255): # Generate left tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        world_tiles[(y*WORLD_WIDTH+x-1)*3] = adj
        
@micropython.native
def generate_right(x, y):
    global world_tiles
    global adjacency_likelihoods
    id = get_tile_id(x, y)
    if((x < WORLD_WIDTH) and get_tile_id(x+1, y) == 255): # Generate right tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        world_tiles[(y*WORLD_WIDTH+x+1)*3] = adj

@micropython.native
def generate_top(x, y):
    global world_tiles
    global adjacency_likelihoods
    id = get_tile_id(x, y)
    if((y > 0) and get_tile_id(x, y-1) == 255): # Generate top tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        world_tiles[((y-1)*WORLD_WIDTH+x)*3] = adj
        
def shuffle(arr):
    for i in range(len(arr)-1, 0 -1):
        j = urandom.randrange(i+1)
        arr[i], arr[j] = arr[j], arr[i]
        
def generate_bottom(x, y):
    global world_tiles
    global adjacency_likelihoods
    id = get_tile_id(x, y)
    if((y < WORLD_HEIGHT) and get_tile_id(x, y+1) == 255): # Generate bottom tile
        sum = adjacency_likelihoods[id][0]
        p = urandom.random()
        adj = 0
        while(sum < p and adj < len(adjacency_likelihoods[id])-1):
            adj += 1
            sum += adjacency_likelihoods[id][adj]
        world_tiles[((y+1)*WORLD_WIDTH+x)*3] = adj
                
def generate_tiles():
    global world_tiles
    fs = [generate_left, generate_right, generate_top, generate_bottom]
    for y in range(0, WORLD_HEIGHT):
        for x in range(0, WORLD_WIDTH):
            if(int(get_tile_id(x, y)) == 255):
                # Generate random tile
                world_tiles[(y*WORLD_WIDTH+x)*3] = urandom.randrange(4)
            shuffle(fs)
            # Each tile only generates two neighbors
            for i in range(3):
                fs[i](x, y)

def generate_water():
    global world_tiles
    for y in range(0, WORLD_HEIGHT):
        for x in range(0, WORLD_WIDTH):
            if(perlin.noise_2d(x*10, y*10) < 0.10):
                world_tiles[(y*WORLD_WIDTH+x)*3] = 4
                
def generate_deco():
    global world_tiles
    for y in range(0, WORLD_HEIGHT):
        for x in range(0, WORLD_WIDTH):
            tile_id = get_tile_id(x, y)
            if(tile_id == 0 or tile_id == 1):
                # 10% chance of having grass decoration
                if(urandom.random() < 0.1):
                    world_tiles[(y*WORLD_WIDTH+x)*3 + 1] = 1
                
                
def draw_minimap():
    # Very slow to generate
    pass
                

urandom.seed()
for y in range(0, WORLD_HEIGHT):
    for x in range(0, WORLD_WIDTH):
        world_tiles[(y*WORLD_WIDTH+x)*3] = 255
        
generate_tiles()
generate_water()
generate_deco()

def move_tiles_right():
    load_renderer_tiles(renderer_x-1, renderer_y)
    camera_tween.start(cam, "position", Vector3(32 + camera_offset.x, camera_offset.y, camera_z), Vector3(camera_offset.x, camera_offset.y, camera_z), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    camera_tween.after = reset_camera_right

def move_tiles_left():
    camera_tween.start(cam, "position", Vector3(camera_offset.x, camera_offset.y, camera_z), Vector3(32 + camera_offset.x, camera_offset.y, camera_z), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    camera_tween.after = reset_camera_left
            
def move_tiles_down():
    load_renderer_tiles(renderer_x, renderer_y-1)
    camera_tween.start(cam, "position", Vector3(camera_offset.x, 32 + camera_offset.y, camera_z), Vector3(camera_offset.x, camera_offset.y, camera_z), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    camera_tween.after = reset_camera_up
            
def move_tiles_up():
    camera_tween.start(cam, "position", Vector3(camera_offset.x, camera_offset.y, camera_z), Vector3(camera_offset.x, 32 + camera_offset.y, camera_z), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    camera_tween.after = reset_camera_down

load_renderer_tiles(renderer_x, renderer_y)
perlin.seed = utime.ticks_us()
#gc.enable()

def action(direction):
    if(control_mode == mode_move):
        if (direction == 0):
            move_tiles_left()
        elif (direction == 1):
            move_tiles_right()
        elif (direction == 2):
            move_tiles_up()
        elif (direction == 3):
            move_tiles_down()
    elif(control_mode == mode_action):
        if (direction == 0 and cursor_pos.x < 32):
            cursor_pos.x += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 1 and cursor_pos.x > -32):
            cursor_pos.x -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 2 and cursor_pos.y < 32):
            cursor_pos.y += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 3 and cursor_pos.y > -32):
            cursor_pos.y -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        
        

while True:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    #player_sprite.position = Vector2(cam.position.x, cam.position.y)
    if engine.tick():
        
        #print(urandom.random())
        #gc.collect()
        print(engine.get_running_fps())
        print(gc.mem_free())
        #print(gc.mem_free())
        #print(str(renderer_x) + ", " + str(renderer_y))
        action_dir = -1
        
        if engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
            action_dir = 0
        elif engine_io.check_just_pressed(engine_io.DPAD_LEFT):
            action_dir = 1
        elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
            action_dir = 2
        elif engine_io.check_just_pressed(engine_io.DPAD_UP):
            action_dir = 3
            
        if engine_io.check_just_pressed(engine_io.A):
            if(control_mode == mode_move):
                control_mode = mode_action
                cursor.position = Vector2(0,0)
                cursor_pos = Vector2(0,0)
                cursor.opacity = 1.0
            elif(control_mode == mode_action):
                control_mode = mode_move
                cursor.opacity = 0.0
        
        if engine_io.check_just_pressed(engine_io.B):
            if(control_mode == mode_move):
                pass
            elif(control_mode == mode_action):
                control_mode = mode_move
                cursor.opacity = 0.0
            
        if(action_dir >= 0):
            action(action_dir)

