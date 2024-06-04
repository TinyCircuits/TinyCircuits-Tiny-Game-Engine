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
# machine.freq(200000000)

# os.chdir("/Games/Thumgeon_II")

import Tiles
import Generate
import Player
import Monsters
import Resources
import Render

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

foo = 0.5
print(gc.mem_free())
current_tilemap = overworld_tiles

print(gc.mem_free())

player_x = Render.renderer_x + 2
player_y = Render.renderer_y + 2

monster_turn = False

def reset_camera_right(self):
    Render.renderer_x -= 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_left(self):
    Render.renderer_x += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_down(self):
    Render.renderer_y += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    
def reset_camera_up(self):
    Render.renderer_y -= 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)

def move_tiles_right():
    global player_x
    player_x -= 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x-1, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x-1, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap, Vector2(1, 0))
    Render.camera_tween.start(Render.cam, "position", Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_right

def move_tiles_left():
    global player_x
    player_x += 1
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_left
            
def move_tiles_down():
    global player_y
    player_y -= 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y-1)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y-1)
    Render.load_renderer_monsters(current_tilemap, Vector2(0, 1))
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_up
            
def move_tiles_up():
    global player_y
    player_y += 1
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_down

print(gc.mem_free())

Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
Render.load_renderer_monsters(current_tilemap)

selection_pos = Vector2(0,0)

def renderer_reload(dummy):
    Render.renderer_x = player_x-2
    Render.renderer_y = player_y-2
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
    Render.load_renderer_monsters(current_tilemap)
    

def draw_inventory():
    item_offset = -64
    for i in player.inventory:
        inventory_sprites.append(Sprite2DNode())
        

class Ghost(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Monsters.monster_textures[4]
        self.frame_count_x = 2
        self.frame_current_x = 0
        self.set_layer(7)
        Render.cam.add_child(self)
        
    def set_frame_count(self, c):
        self.frame_count_x = c
        self.frame_current_x = 0

def update_monsters():
    print("Updating monsters")
    global current_tilemap
    for m in current_tilemap.monster_list:
        dx = m.position.x - player_x
        dy = m.position.y - player_y
        if(dx*dx + dy*dy > 8*8):
            # Despawn monsters far from the player
            print("Despawning mob!")
            current_tilemap.monster_list.remove(m)
            del m
            continue
        elif(abs(dx) > 1 or abs(dy) > 1):
            old_pos = Vector2(m.position.x, m.position.y)
            if(abs(dx) > abs(dy)):
                if(dx < 0):
                    m.position.x += 1
                else:
                    m.position.x -= 1
            else:
                if(dy < 0):
                    m.position.y += 1
                else:
                    m.position.y -= 1
            for m2 in current_tilemap.monster_list:
                if(m != m2):
                    if(m.position.x == m2.position.x and m.position.y == m2.position.y):
                        m.position = old_pos
                        break
        else:
            # Monster close enough to hit the player
            player.hp -= urandom.randrange(Monsters.monster_dmg_range[m.id][0], Monsters.monster_dmg_range[m.id][1])
    if(len(current_tilemap.monster_list) < 5 and urandom.random() < 0.05):
        print("Spawning mob!")
        # Spawn a monster some distance from the player
        dx = urandom.randrange(4, 6)
        if(urandom.random() < 0.5):
            dx = -dx
        dy = urandom.randrange(4, 6)
        if(urandom.random() < 0.5):
            dy = -dy
        m = Monsters.Monster()
        m.set_monster(urandom.choice(list(Monsters.monster_ids.values())))
        m.position = Vector2(player_x + dx, player_y + dy)
        print(m.position)
        current_tilemap.monster_list.append(m)
    #Render.load_renderer_monsters(current_tilemap)
        
        

def tile_action(x, y):
    global current_tilemap
    global dungeon_tiles
    global dungeon_egress
    global current_dungeon_level
    global player_x
    global player_y
    global tiles_redraw
    global monster_redraw
    global deco_redraw
    print(str(2 + selection_pos.x) + ", " + str(2 + selection_pos.y))
    Render.tile_animate_action(int(2 + selection_pos.x), int(2 + selection_pos.y), renderer_reload)
    #Render.tile_animate_action(int(2 + selection_pos.x), int(2 + selection_pos.y), None)
    if((current_tilemap.get_tile_data1(x, y) & (1 << 2)) == 0):
        if(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["door_sheet"]):
            print("Acted on door!")
            if(len(dungeon_egress) == 0):
                print("Loading level "+str(current_dungeon_level))
                dungeon_egress.append(Vector2(player_x, player_y))
                current_tilemap = dungeon_levels[current_dungeon_level]
                Render.renderer_x = current_tilemap.entryway.x - 2
                Render.renderer_y = current_tilemap.entryway.y - 2
                player_x = current_tilemap.entryway.x
                player_y = current_tilemap.entryway.y
                current_dungeon_level += 1
            else:
                if(current_dungeon_level == 1):
                    current_tilemap = overworld_tiles
                player_x = dungeon_egress[len(dungeon_egress)-1].x
                player_y = dungeon_egress[len(dungeon_egress)-1].y
                Render.renderer_x = player_x - 2
                Render.renderer_y = player_y - 2
                dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
                current_dungeon_level = 0
        elif(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["trapdoor_sheet"]):
            print("Acted on trapdoor")
            dungeon_egress.append(Vector2(Render.renderer_x, Render.renderer_y))
            print("Loading level "+str(current_dungeon_level))
            current_tilemap = dungeon_levels[current_dungeon_level]
            Render.renderer_x = current_tilemap.entryway.x - 2
            Render.renderer_y = current_tilemap.entryway.y - 2
            player_x = current_tilemap.entryway.x
            player_y = current_tilemap.entryway.y
            current_dungeon_level += 1
        elif(current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["ladder_sheet"]):
            print("Acted on ladder")
            print("Loading level "+str(current_dungeon_level-2))
            current_tilemap = dungeon_levels[current_dungeon_level-2]
            Render.renderer_x = dungeon_egress[len(dungeon_egress)-1].x
            Render.renderer_y = dungeon_egress[len(dungeon_egress)-1].y
            player_x = Render.renderer_x + 2
            player_y = Render.renderer_y + 2
            dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
            current_dungeon_level -= 1
        else:
            for m in current_tilemap.monster_list:
                if(m.position.x == x and m.position.y == y):
                    print("Attacking monster with hp " + str(m.hp))
                    m.hp -= 2
                    if(m.hp <= 0):
                        current_tilemap.monster_list.remove(m)
                    deco_redraw = True
            monster_redraw = True
            #tiles_redraw = True
            
            print("Other action")
    else:
        # Item tile
        print("Picking up item id " + str(current_tilemap.get_tile_data0(x, y)))
        print("Player wt is " + str(player.wt))
        item = current_tilemap.get_tile_data0(x, y)
        if(item == Player.item_ids["gold_coins"] or item == Player.item_ids["gold_bars"]):
            player.gp += urandom.randrange(5, 12)
            current_tilemap.set_tile_data0(x, y, 0)
            current_tilemap.set_deco_item(x, y, False)
        elif(player.add_inv_item(item)):
            current_tilemap.set_tile_data0(x, y, 0)
            current_tilemap.set_deco_item(x, y, False)
        else:
            print("Too heavy!")
            
        pass
    
    
    #Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    #renderer_reload(None)

def action(direction):
    global player_x
    global player_y
    global selection_pos
    global monster_turn
    if(control_mode == mode_move):
        if (direction == 0):
            # Try to move rightward
            if((current_tilemap.get_tile_data1(int(player_x+1), int(player_y)) & 0x1) == 0):
                move_tiles_left() # Right tile is nonsolid
                monster_turn = True
        elif (direction == 1):
            # Try to move leftward
            if((current_tilemap.get_tile_data1(int(player_x-1), int(player_y)) & 0x1) == 0):
                move_tiles_right() # Left tile is nonsolid
                monster_turn = True
        elif (direction == 2):
             # Try to move downward
            if((current_tilemap.get_tile_data1(int(player_x), int(player_y+1)) & 0x1) == 0):
                move_tiles_up() # Down tile is nonsolid
                monster_turn = True
        elif (direction == 3):
            # Try to move upward
            if((current_tilemap.get_tile_data1(int(player_x), int(player_y-1)) & 0x1) == 0):
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
        self.text = "test"
        self.set_layer(7)
        self.color = Color(0x0000)
        self.position = Vector2(0, -48)
        self.font = Resources.roboto_font
        self.opacity = 1.0
        Render.cam.add_child(self)

def set_cursor():
    cursor.texture = Tiles.action_indicator
    for m in current_tilemap.monster_list:
        if(player_x + selection_pos.x == m.position.x and player_y + selection_pos.y == m.position.y):
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

hp_text.font = Resources.roboto_font
mp_text.font = Resources.roboto_font

hp_text.text = "HP: "+str(player.maxhp)+" / "+str(player.maxhp)
mp_text.text = "MP: "+str(player.maxmp)+" / "+str(player.maxmp)
        
hp_text.set_layer(7)
mp_text.set_layer(7)

Render.cam.add_child(hp_text)
Render.cam.add_child(mp_text)

inventory_item_sel = 0

tiles_redraw = False
monster_redraw = True
deco_redraw = False

print("Entering game loop")
    
while True:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():
        #print(1)
        
        #print(urandom.random())
        #gc.collect()
        #print(engine.get_running_fps())
        #print(str(len(current_tilemap.monster_list)) + " monsters alive")
        #print(gc.mem_free())
        #print(gc.mem_free())
        #print(str(player_x) + ", " + str(player_y))
        #print(str(Render.renderer_x + 2 + selection_pos.x) + ", " + str(Render.renderer_y + 2 + selection_pos.y))
        action_dir = -1
        
        monster_turn = False
        tiles_redraw = False
        monster_redraw = False
        deco_redraw = False
        
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
                print("acting on "+str(selection_pos.x + player_y) + ", " + str(selection_pos.y + player_y))
                tile_action(int(selection_pos.x + player_x), int(selection_pos.y + player_y))
                control_mode = mode_move
                cursor.opacity = 0.0
                #Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
                monster_turn = True
                #monster_redraw = True
        
        if engine_io.check_just_pressed(engine_io.B):
            if(control_mode == mode_move):
                control_mode = mode_inventory
                inventory_item_sel = 0
                pass
            elif(control_mode == mode_action or control_mode == mode_inventory):
                player.held_item_spr.frame_current_x = 0
                control_mode = mode_move
                cursor.opacity = 0.0
            
        if(action_dir >= 0):
            action(action_dir)
            
        if(tiles_redraw):
            Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
        if(deco_redraw):
            Render.load_renderer_deco(current_tilemap, Render.renderer_x, Render.renderer_y)
        if(monster_redraw):
            Render.load_renderer_monsters(current_tilemap)
        if(monster_turn):
            update_monsters()
        
        look_ang = 0.0
        player.scale = Vector2(1.0, 1.0)
        if(control_mode == mode_action):
            look_ang = math.atan2(-cursor.position.y, cursor.position.x)
        player.look_ang = look_ang
        
        set_cursor()
        
        hp_text.text = "HP: "+str(player.hp) + " / " + str(player.maxhp)
        mp_text.text = "MP: "+str(player.mp) + " / " + str(player.maxmp)

        inventory_renderer.sprlist.clear()
        inventory_renderer.destroy_children()

        if(control_mode == mode_inventory):
            if(action_dir == 0):
                inventory_item_sel += 1
                if(inventory_item_sel >= len(player.inventory)):
                    inventory_item_sel = len(player.inventory)-1
            elif(action_dir == 1):
                inventory_item_sel -= 1
                if(inventory_item_sel < 0):
                    inventory_item_sel = 0
            gp_text = Text2DNode()
            wt_text = Text2DNode()
            
            gp_text.text = "GP: "+str(player.gp)
            wt_text.text = "WT: "+str(player.wt)+" / "+str(player.maxwt)
            
            gp_text.font = Resources.roboto_font
            wt_text.font = Resources.roboto_font
            
            gp_text.position =  Vector2(32, -4)
            wt_text.position =  Vector2(32, 4)
            
            gp_text.set_layer(7)
            wt_text.set_layer(7)
            
            inventory_renderer.add_child(gp_text)
            inventory_renderer.add_child(wt_text)
            
            item_spr_offset = Vector2(-inventory_item_sel*32, -32)
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
                
        #ghost.set_frame_count(1)
                


