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
import engine_debug
import math
import os
import gc
import urandom
import utime
import machine
import math

gc.threshold(1024*32)
#gc.disable()
engine_physics.set_gravity(0.0, 0.0)
engine.set_fps_limit(50)
#engine.disable_fps_limit()
machine.freq(200000000)

os.chdir("/Games/Thumgeon_II")
roboto_font = FontResource("9pt-roboto-font.bmp", True)

import Tiles
import Render
import Generate
import Player
import Monsters

print(os.getcwd())

player = Player.Player()

#engine_debug.enable_all()

class CursorSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Tiles.action_indicator
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.set_layer(7)
        self.opacity = 0.0
        self.transparent_color = Color(0x07e0)
        self.width = 32
        self.height = 32
        self.tween = Tween()
        Render.cam.add_child(self)
        
cursor = CursorSprite()
cursor_pos = Vector2(0, 0)

mode_move = 0
mode_action = 1
mode_inventory = 2

control_mode = mode_move
    
tween_snap = 80 # Controls camera tween speed
indicator_snap = 50 # Controls indicator tween speed

camera_z = 0.0


def draw_minimap():
    # Very slow to generate
    pass

overworld_tiles = Tiles.Tilemap(48, 48)

urandom.seed()
for y in range(0, overworld_tiles.HEIGHT):
    for x in range(0, overworld_tiles.WIDTH):
        overworld_tiles.tiles[(y*overworld_tiles.WIDTH+x)*3] = 255

overworld_tiles.border_tile = Tiles.tile_ids["water1"]
Generate.generate_tiles(overworld_tiles)
#Generate.generate_water(overworld_tiles)
Generate.generate_deco(overworld_tiles)

overworld_tiles.monster_list.append(Monsters.Monster())

dungeon_levels = []
current_dungeon_level = 0
for i in range(5):
    dungeon_levels.append(Tiles.Tilemap(urandom.randrange(5, 12), urandom.randrange(5, 12)))
    Generate.generate_empty_dungeon(dungeon_levels[i], True if i == 0 else False)
    Generate.generate_dungeon_level(dungeon_levels[i], True if i > 0 else False, True if i < 4 else False)

dungeon_tiles = dungeon_levels[0]

dungeon_egress = []

#Generate.generate_empty_dungeon(dungeon_tiles)
#Generate.generate_dungeon_level(dungeon_tiles)

#current_dungeon_level = 0
#print(gc.mem_free())
foo = 0.5
print(gc.mem_free())
current_tilemap = overworld_tiles

print(gc.mem_free())
#current_tilemap = dungeon_tiles

def reset_camera_right(self):
    Render.renderer_x -= 1
    #Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_left(self):
    Render.renderer_x += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_down(self):
    Render.renderer_y += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_up(self):
    Render.renderer_y -= 1
    #Render.load_renderer_monsters(current_tilemap)

def move_tiles_right():
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x-1, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap, Vector2(1,0))
    Render.camera_tween.start(Render.cam, "position", Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_right

def move_tiles_left():
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_left
    
            
def move_tiles_down():
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y-1)
    Render.load_renderer_monsters(current_tilemap, Vector2(0,1))
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_up
            
def move_tiles_up():
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_down

print(gc.mem_free())

Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)

selection_pos = Vector2(0,0)

def renderer_reload(dummy):
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    #Render.anim_tween.restart()

def draw_inventory():
    item_offset = -64
    for i in player.inventory:
        inventory_sprites.append(Sprite2DNode())
        
def update_monsters():
    global current_tilemap
    for m in current_tilemap.monster_list:
        dx = m.position.x - (Render.renderer_x + 2)
        dy = m.position.y - (Render.renderer_y + 2)
        if(dx*dx + dy*dy > 8*8):
            # Despawn monsters far from the player
            print("Despawning mob!")
            current_tilemap.monster_list.remove(m)
            continue
    if(len(current_tilemap.monster_list) < 5 and urandom.random() < 0.1):
        print("Spawning mob!")
        # Spawn a monster some distance from the player
        dx = urandom.randrange(4, 6)
        if(urandom.random() < 0.5):
            dx = -dx
        dy = urandom.randrange(4, 6)
        if(urandom.random() < 0.5):
            dy = -dy
        m = Monsters.Monster()
        m.texture = urandom.choice(list(Monsters.monster_textures.values()))
        m.position = Vector2(Render.renderer_x + dx + 2, Render.renderer_y + dy + 2)
        print(m.position)
        current_tilemap.monster_list.append(m)
        

def tile_action(x, y):
    global current_tilemap
    global dungeon_tiles
    global dungeon_egress
    global current_dungeon_level
    print(str(2 + selection_pos.x) + ", " + str(2 + selection_pos.y))
    Render.tile_animate_action(int(2 + selection_pos.x), int(2 + selection_pos.y), renderer_reload)
    if((current_tilemap.get_tile_data1(x, y) & (1 << 2)) == 0):
        if(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["door_sheet"]):
            print("Acted on door!")
            if(len(dungeon_egress) == 0):
                print("Loading level "+str(current_dungeon_level))
                dungeon_egress.append(Vector2(Render.renderer_x, Render.renderer_y))
                current_tilemap = dungeon_levels[current_dungeon_level]
                Render.renderer_x = current_tilemap.entryway.x - 2
                Render.renderer_y = current_tilemap.entryway.y - 2
                current_dungeon_level += 1
            else:
                if(current_dungeon_level == 1):
                    current_tilemap = overworld_tiles
                Render.renderer_x = dungeon_egress[len(dungeon_egress)-1].x
                Render.renderer_y = dungeon_egress[len(dungeon_egress)-1].y
                dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
                current_dungeon_level = 0
        elif(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["trapdoor_sheet"]):
            print("Acted on trapdoor")
            dungeon_egress.append(Vector2(Render.renderer_x, Render.renderer_y))
            print("Loading level "+str(current_dungeon_level))
            current_tilemap = dungeon_levels[current_dungeon_level]
            Render.renderer_x = current_tilemap.entryway.x - 2
            Render.renderer_y = current_tilemap.entryway.y - 2
            current_dungeon_level += 1
        elif(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["ladder_sheet"]):
            print("Acted on ladder")
            print("Loading level "+str(current_dungeon_level-2))
            current_tilemap = dungeon_levels[current_dungeon_level-2]
            Render.renderer_x = dungeon_egress[len(dungeon_egress)-1].x
            Render.renderer_y = dungeon_egress[len(dungeon_egress)-1].y
            dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
            current_dungeon_level -= 1
        else:
            print("Other action")
    else:
        # Item tile
        print("Picking up item id " + str(current_tilemap.get_tile_data0(x, y)))
        print("Player wt is " + str(player.wt))
        item = current_tilemap.get_tile_data0(x, y)
        if(player.add_inv_item(item)):
            current_tilemap.set_tile_data0(x, y, 0)
            current_tilemap.set_deco_item(x, y, False)
        else:
            print("Too heavy!")
        pass
    
    
    #Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    #renderer_reload(None)

def action(direction):
    px = Render.renderer_x+2
    py = Render.renderer_y+2
    global selection_pos
    global monster_turn
    if(control_mode == mode_move):
        if (direction == 0):
            # Try to move rightward
            if((current_tilemap.get_tile_data1(int(px+1), int(py)) & 0x1) == 0):
                move_tiles_left() # Right tile is nonsolid
                monster_turn = True
        elif (direction == 1):
            # Try to move leftward
            if((current_tilemap.get_tile_data1(int(px-1), int(py)) & 0x1) == 0):
                move_tiles_right() # Left tile is nonsolid
                monster_turn = True
        elif (direction == 2):
             # Try to move downward
            if((current_tilemap.get_tile_data1(int(px), int(py+1)) & 0x1) == 0):
                move_tiles_up() # Down tile is nonsolid
                monster_turn = True
        elif (direction == 3):
            # Try to move upward
            if((current_tilemap.get_tile_data1(int(px), int(py-1)) & 0x1) == 0):
                move_tiles_down()
                monster_turn = True
    elif(control_mode == mode_action):
        if (direction == 0 and cursor_pos.x < 32):
            selection_pos.x += 1
            cursor_pos.x += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 1 and cursor_pos.x > -32):
            selection_pos.x -= 1
            cursor_pos.x -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 2 and cursor_pos.y < 32):
            selection_pos.y += 1
            cursor_pos.y += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 3 and cursor_pos.y > -32):
            selection_pos.y -= 1
            cursor_pos.y -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)

class MessageSprite(Text2DNode):
    def __init__(self):
        super().__init__(self)
#         self.text = "test"
#         self.set_layer(7)
#         self.color = Color(0x0000)
#         self.position = Vector2(0, -48)
#         self.font = roboto_font
        self.opacity = 0.0
#         Render.cam.add_child(self)

def set_cursor():
    cursor.texture = Tiles.action_indicator
    for m in current_tilemap.monster_list:
        if(Render.renderer_x + 2 + selection_pos.x == m.position.x and Render.renderer_y + 2 + selection_pos.y == m.position.y):
            cursor.texture = Tiles.attack_indicator
    
print(gc.mem_free())
    
current_msg = MessageSprite()

class InventoryRenderer(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.sprlist = []

print("Making nodes")

inventory_renderer = InventoryRenderer()

Render.cam.add_child(inventory_renderer)

hp_text = Text2DNode()
mp_text = Text2DNode()

hp_text.position =  Vector2(-32, 40)
mp_text.position =  Vector2(-32, 56)

hp_text.font = roboto_font
mp_text.font = roboto_font

hp_text.text = "HP: "+str(player.maxhp)+" / "+str(player.maxhp)
mp_text.text = "MP: "+str(player.maxmp)+" / "+str(player.maxmp)
        
hp_text.set_layer(7)
mp_text.set_layer(7)

Render.cam.add_child(hp_text)
Render.cam.add_child(mp_text)

print("Entering game loop")
    
while True:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():
        #print(1)
        
        #print(urandom.random())
        #gc.collect()
        print(engine.get_running_fps())
        print(str(len(current_tilemap.monster_list)) + " monsters alive")
        #print(gc.mem_free())
        #print(gc.mem_free())
        #print(str(Render.renderer_x) + ", " + str(Render.renderer_y))
        #print(str(Render.renderer_x + 2 + selection_pos.x) + ", " + str(Render.renderer_y + 2 + selection_pos.y))
        action_dir = -1
        
        monster_turn = False
        
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
                if(player.held_item_spr.frame_count_x > 1):
                    player.held_item_spr.frame_current_x = 1
                control_mode = mode_action
                cursor.position = Vector2(0,0)
                cursor_pos = Vector2(0,0)
                selection_pos = Vector2(0,0)
                cursor.opacity = 1.0
            elif(control_mode == mode_action):
                player.held_item_spr.frame_current_x = 0
                tile_action(int(2 + selection_pos.x + Render.renderer_x), int(2 + selection_pos.y + Render.renderer_y))
                control_mode = mode_move
                cursor.opacity = 0.0
                monster_turn = True
        
        if engine_io.check_just_pressed(engine_io.B):
            if(control_mode == mode_move):
                control_mode = mode_inventory
                pass
            elif(control_mode == mode_action or control_mode == mode_inventory):
                player.held_item_spr.frame_current_x = 0
                control_mode = mode_move
                cursor.opacity = 0.0
            
        if(action_dir >= 0):
            action(action_dir)
            
        if(monster_turn):
            update_monsters()
        
        look_ang = 0.0
        player.scale = Vector2(1.0, 1.0)
        if(control_mode == mode_action):
            look_ang = math.atan2(-cursor.position.y, cursor.position.x)
        player.look_ang = look_ang
        
        set_cursor()
        
        hp_text.text = "HP: "+str(player.hp)+" / "+str(player.maxhp)
        mp_text.text = "MP: "+str(player.mp)+" / "+str(player.maxmp)
    
        inventory_renderer.sprlist.clear()
        inventory_renderer.destroy_children()

        if(control_mode == mode_inventory):
            gp_text = Text2DNode()
            wt_text = Text2DNode()
            
            gp_text.text = "GP: "+str(player.gp)
            wt_text.text = "WT: "+str(player.wt)+" / "+str(player.maxwt)
            
            gp_text.font = roboto_font
            wt_text.font = roboto_font
            
            gp_text.position =  Vector2(32, -4)
            wt_text.position =  Vector2(32, 4)
            
            gp_text.set_layer(7)
            wt_text.set_layer(7)
            
            inventory_renderer.add_child(gp_text)
            inventory_renderer.add_child(wt_text)
            
            item_spr_offset = Vector2(0, -32)
            for i in player.inventory:
                
                frame_spr = Sprite2DNode()
                frame_spr.texture = Player.frame_texture
                frame_spr.position.x = item_spr_offset.x
                frame_spr.position.y = item_spr_offset.y
                frame_spr.playing = False
                frame_spr.transparent_color  = Color(0x07e0)
                frame_spr.set_layer(6)
                frame_spr.frame_count_x = 1
                frame_spr.frame_current_x = 0
                
                item_spr = Sprite2DNode()
                item_spr.texture = i.texture
                item_spr.position.x = item_spr_offset.x
                item_spr.position.y = item_spr_offset.y
                item_spr.playing = False
                item_spr.transparent_color  = Color(0x07e0)
                item_spr.set_layer(7)
                item_spr.frame_count_x = i.frame_count
                item_spr.frame_current_x = 0
                item_spr_offset.x += 32
                inventory_renderer.sprlist.append(frame_spr)
                inventory_renderer.sprlist.append(item_spr)
                inventory_renderer.add_child(frame_spr)
                inventory_renderer.add_child(item_spr)
                
                
        #Render.load_renderer_monsters()
                


