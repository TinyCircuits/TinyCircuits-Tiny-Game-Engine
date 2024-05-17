import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN
import math
import os
import gc

print(os.getcwd())

grass1 = TextureResource("GrassTile1_16bit.bmp")
grass2 = TextureResource("GrassTile2_16bit.bmp")


engine_physics.set_gravity(0, 0.0)
engine.set_fps_limit(120)
# machine.freq(250000000)

class Tile(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = grass1
        self.frame_count_x = 1
        self.width = 32
        self.height = 32
        self.id = 0
        self.tween = Tween()
        #self.tween.start(self.tile.rotation, "", self.tile.rotation, 2*math.pi, 2000.0, 1.0, PING_PONG, EASE_ELAST_IN_OUT)
    
    def collision(self, contact):
        #self.tween.stop()
        pass

class TileRenderer(EmptyNode):
    def __init__(self):
        super().__init__(self)
        self.tween = Tween()
        self.tiles = [None]*9*9
        self.offset = Vector2(-64, -64)
        self.position = Vector2(self.offset.x, self.offset.y)
        for x in range(0, 9):
            for y in range(0, 9):
                self.tiles[y*9+x] = Tile()
                self.tiles[y*9+x].position = Vector2(32*(x-1), 32*(y-1))
                self.add_child(self.tiles[y*9 +x])
                
        #self.tiles[y*8+8].texture = 
        
#tile_renderer = TileRenderer()

renderer_tiles = [None] * 9 * 9
for x in range(0, 9):
    for y in range(0, 9):
        renderer_tiles[y*9+x] = Tile()
        renderer_tiles[y*9+x].position = Vector2(32*(x-1), 32*(y-1))

WORLD_WIDTH = const(128)
WORLD_HEIGHT = const(128)
world_tiles = bytearray(WORLD_WIDTH*WORLD_HEIGHT*3)

def get_tile_id(x, y):
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3]

def get_tile_data0(x, y):
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3+1]

def get_tile_data1(x, y):
    if((x < 0) or (x > WORLD_WIDTH-1) or (y < 0) or (y > WORLD_HEIGHT - 1)):
        return -1
    return world_tiles[(y*WORLD_WIDTH+x)*3+2]

def tile_id_to_sprite(tile_id):
    global grass1
    global grass2
    if(tile_id == 0):
        return grass1
    elif(tile_id == 1):
        return grass2
    else:
        return None
        
cam = CameraNode()
camera_tween = Tween()
camera_offset = Vector2(64, 64)

renderer_x = -2
renderer_y = -2

def reset_camera_right(self):
    global renderer_x
    renderer_x -= 1
    cam.position = Vector3(64,64,0)
    #load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_left(self):
    global renderer_x
    renderer_x += 1
    cam.position = Vector3(64,64,0)
    load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_down(self):
    global renderer_y
    renderer_y += 1
    cam.position = Vector3(64,64,0)
    load_renderer_tiles(renderer_x, renderer_y)
    
def reset_camera_up(self):
    global renderer_y
    renderer_y -= 1
    cam.position = Vector3(64,64,0)
    load_renderer_tiles(renderer_x, renderer_y)

def reset_camera(self):
    global renderer_x
    global renderer_y
    cam.position = Vector3(64,64,0)

@micropython.native
def load_renderer_tiles(cx, cy):
    for y in range(0, 9):
        for x in range(0, 9):
            renderer_tiles[y*9+x].id = get_tile_id(cx+x-1, cy+y-1)
            renderer_tiles[y*9+x].texture = tile_id_to_sprite(renderer_tiles[y*9+x].id)
            
    reset_camera(None)
    
tween_snap = 120

for y in range(0, WORLD_HEIGHT):
    for x in range(0, WORLD_WIDTH):
        world_tiles[(y*WORLD_WIDTH+x)*3] = 0

def move_tiles_right():
    load_renderer_tiles(renderer_x-1, renderer_y)
    camera_tween.start(cam, "position", Vector3(32 + camera_offset.x, camera_offset.y, 0), Vector3(camera_offset.x, camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_SINE_IN)
    camera_tween.after = reset_camera_right

def move_tiles_left():
    camera_tween.start(cam, "position", Vector3(camera_offset.x, camera_offset.y, 0), Vector3(32 + camera_offset.x, camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_SINE_IN)
    camera_tween.after = reset_camera_left
            
def move_tiles_down():
    load_renderer_tiles(renderer_x, renderer_y-1)
    camera_tween.start(cam, "position", Vector3(camera_offset.x, 32 + camera_offset.y, 0), Vector3(camera_offset.x, camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_SINE_IN)
    camera_tween.after = reset_camera_up

            
def move_tiles_up():
    camera_tween.start(cam, "position", Vector3(camera_offset.x, camera_offset.y, 0), Vector3(camera_offset.x, 32 + camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_SINE_IN)
    camera_tween.after = reset_camera_down
    
world_tiles[0] = 1

load_renderer_tiles(renderer_x, renderer_y)

#gc.enable()
while True:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():
        print(engine.get_running_fps())
        print(gc.mem_free())
        print(str(cam.position.x) + ", " + str(cam.position.y))
        print(str(renderer_x) + ", " + str(renderer_y))
        #gc.collect()
        #print(gc.mem_free())
        if engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
            move_tiles_left()
            pass
        if engine_io.check_just_pressed(engine_io.DPAD_LEFT):
            move_tiles_right()
            pass
        if engine_io.check_just_pressed(engine_io.DPAD_DOWN):
            move_tiles_up()
            #load_renderer_tiles(renderer_x, renderer_y)
            pass
        if engine_io.check_just_pressed(engine_io.DPAD_UP):
            move_tiles_down()
            #load_renderer_tiles(renderer_x, renderer_y)
            pass

