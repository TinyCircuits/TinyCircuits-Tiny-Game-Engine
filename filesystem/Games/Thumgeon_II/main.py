import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
import engine_debug
from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource, NoiseResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN, EASE_LINEAR
import math
import os
import gc
import urandom
import utime
import machine
import math

machine.freq(250000000)

gc.threshold(1024*32)
engine_physics.set_gravity(0.0, 0.0)
engine.fps_limit(60)

import Tiles
import Generate
import Player
import Monsters
import Resources
import Render

print(os.getcwd())

player = Player.Player()

# engine_debug.enable_all()
splash_sprite = Sprite2DNode(position=Vector2(0, 0), texture=Resources.splash_texture)

splash_sprite.layer = 7

Render.cam.add_child(splash_sprite)

while(not engine_io.A.is_just_pressed and not engine_io.B.is_just_pressed):
    engine.tick()

splash_sprite.mark_destroy_all()

class CursorSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Tiles.action_indicator
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.layer = 7
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
mode_shop = 3

control_mode = mode_move

tween_snap = 80 # Controls camera tween speed
indicator_snap = 50 # Controls indicator tween speed

camera_z = 0.0

overworld_tiles = Tiles.Tilemap(32, 32)

urandom.seed()
for y in range(0, overworld_tiles.HEIGHT):
    for x in range(0, overworld_tiles.WIDTH):
        overworld_tiles.tiles[(y*overworld_tiles.WIDTH+x)*Tiles.TILE_DATA_BYTES] = 255

overworld_tiles.border_tile = Tiles.tile_ids["water1"]
Generate.generate_tiles(overworld_tiles)
#Generate.generate_water(overworld_tiles)
Generate.generate_deco(overworld_tiles)


current_dungeon_level = 0
overworld_tiles.monster_list.append(Monsters.Monster())
dungeon_egress = []

print(gc.mem_free())

current_dungeon = None

current_tilemap = overworld_tiles

print(gc.mem_free())

spawn_pos = Generate.get_free_pos(current_tilemap, 3, 29, 3, 29)

Render.renderer_x = spawn_pos.x
Render.renderer_y = spawn_pos.y

player_x = Render.renderer_x + 2
player_y = Render.renderer_y + 2

monster_turn = False

def reset_camera_right(self):
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap)

def reset_camera_left(self):
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap)

def reset_camera_down(self):
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap)

def reset_camera_up(self):
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap)

def move_tiles_right():
    global player_x
    player_x -= 1
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap, Vector2(1, 0))
    Render.renderer_x -= 1
    Render.camera_tween.start(Render.cam, "position", Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_right

def move_tiles_left():
    global player_x
    player_x += 1
    Render.renderer_x += 1
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_left

def move_tiles_down():
    global player_y
    player_y -= 1
    Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
    Render.load_renderer_monsters(current_tilemap, Vector2(0, 1))
    Render.renderer_y -= 1
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_up

def move_tiles_up():
    global player_y
    player_y += 1
    Render.renderer_y += 1
    Render.camera_tween.start(Render.cam, "position", Vector3(Render.camera_offset.x, Render.camera_offset.y, 0), Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0), tween_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
    Render.camera_tween.after = reset_camera_down

Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
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
    for i in inventory_renderer.inv:
        inventory_sprites.append(Sprite2DNode())


class Ghost(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Monsters.monster_textures[4]
        self.frame_count_x = 2
        self.frame_current_x = 0
        self.layer = 7
        Render.cam.add_child(self)

    def set_frame_count(self, c):
        self.frame_count_x = c
        self.frame_current_x = 0

class MessageSprite(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.text = ""
        self.layer = 7
        self.color = Color(0xFFFF)
        self.position = Vector2(0, -20)
        self.font = Resources.roboto_font
        self.opacity = 1.0
        Render.cam.add_child(self)

current_msg = MessageSprite()

def update_monsters():
    print("Updating monsters")
    global current_tilemap
    for m in current_tilemap.monster_list:
        dx = m.position.x - player_x
        dy = m.position.y - player_y
        print("Updating monster with stun "+str(m.stun))
        if(m.stun == 0 and m.id == Monsters.monster_ids["litbomb"]):
            if(abs(dx) <= 1 and abs(dy) <= 1):
                dmg = urandom.randrange(Monsters.monster_dmg_range[m.id][0], Monsters.monster_dmg_range[m.id][1])
                player.hp -= dmg
                start_hitmarker(hitmarkers[1], 0, 0, dmg)
                name = {i for i in Monsters.monster_ids if Monsters.monster_ids[i] == m.id}
                if(player.hp <= 0):
                    show_death_screen("Killed by bomb")
            for m2 in current_tilemap.monster_list:
                if(m2 != m):
                    dx = m.position.x - m2.position.x
                    dy = m.position.y - m2.position.y
                    if(abs(dx) <= 1 and abs(dy) <= 1):
                        dmg = urandom.randrange(Monsters.monster_dmg_range[m.id][0], Monsters.monster_dmg_range[m.id][1])
                        m2.hp -= dmg
                        start_hitmarker(hitmarkers[0], m2.position.x, m2.position.y, dmg)
                        if(m2.hp <= 0):
                            current_tilemap.monster_list.remove(m2)
            current_tilemap.monster_list.remove(m)
            m.mark_destroy_children()
            m.mark_destroy()
        elif(dx*dx + dy*dy > 8*8):
            print("Despawning mob!")
            m.mark_destroy_children()
            m.mark_destroy()
            current_tilemap.monster_list.remove(m)
            continue
        elif(m.stun > 0):
            m.stun -= 1
            if(m.stun == 0 and m.id == Monsters.monster_ids["litbomb"]):
                m.frame_current_x = 2
        elif(abs(dx) > 1 or abs(dy) > 1):
            old_pos = Vector2(m.position.x, m.position.y)
            if(abs(dx) > abs(dy)):
                if(dx < 0 and not current_tilemap.tile_solid(m.position.x+1, m.position.y) and
                not current_tilemap.tile_occupied(m.position.x + 1, m.position.y)):
                    m.position.x += 1
                elif(dx > 0 and not current_tilemap.tile_solid(m.position.x-1, m.position.y) and
                not current_tilemap.tile_occupied(m.position.x - 1, m.position.y)):
                    m.position.x -= 1
            else:
                if(dy < 0 and not current_tilemap.tile_solid(m.position.x, m.position.y + 1) and
                not current_tilemap.tile_occupied(m.position.x, m.position.y + 1)):
                    m.position.y += 1
                elif(dy > 0 and not current_tilemap.tile_solid(m.position.x, m.position.y-1) and
                not current_tilemap.tile_occupied(m.position.x, m.position.y - 1)):
                    m.position.y -= 1
        else:
            # Monster close enough to hit the player
            dmg = urandom.randrange(Monsters.monster_dmg_range[m.id][0], Monsters.monster_dmg_range[m.id][1])
            player.hp -= dmg
            start_hitmarker(hitmarkers[1], 0, 0, dmg)
            if(player.hp <= 0):
                name = {i for i in Monsters.monster_ids if Monsters.monster_ids[i] == m.id}
                if(player.hp <= 0):
                    show_death_screen("Killed by "+str(list(name)[0]))
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
        m.set_monster(urandom.choice(current_tilemap.spawn_list))
        m.position = Vector2(player_x + dx, player_y + dy)
        current_tilemap.monster_list.append(m)

inventory_item_sel = 0
inventory_ui_sel = 0

hitmarker_text = Text2DNode()
hitmarker_text.font = Resources.roboto_font
hitmarker_text.text = ""
hitmarker_text.opacity = 0.0
hitmarker_text.position = Vector2(0, 0)
hitmarker_text.color = engine_draw.red
Render.cam.add_child(hitmarker_text)

class Hitmarker(Text2DNode):
    def __init__(self, value, x, y):
        super().__init__(self)
        self.text = str(value)
        self.position = Vector2(32*x + 8, 32*y)
        self.color = engine_draw.red
        self.opacity = 1.0
        self.font = Resources.roboto_font
        self.layer = 7
        self.pos_tween = Tween()
        self.opacity_tween = Tween()

    def start(self, x, y, value):
        self.text = str(value)
        self.opacity_tween.start(self, "opacity", 1.0, 0.0, 800, 1.0, ONE_SHOT, EASE_QUAD_IN)
        self.pos_tween.start(self, "position", Vector2(32*x + 8, 32*y), Vector2(32*x + 8, 32*y-24), 300, 1.0, ONE_SHOT, EASE_LINEAR)

def start_hitmarker(marker, x, y, value):
    marker.text = str(value)
    marker.opacity_tween.start(marker, "opacity", 1.0, 0.0, 800, 1.0, ONE_SHOT, EASE_QUAD_IN)
    marker.pos_tween.start(marker, "position", Vector2(32*x + 8, 32*y), Vector2(32*x + 8, 32*y-24), 300, 1.0, ONE_SHOT, EASE_LINEAR)


hitmarkers = [Hitmarker(0, 0, 0), Hitmarker(0, 0, 0), Hitmarker(0, 0, 0)]

for h in hitmarkers:
    h.opacity = 0.0
    Render.cam.add_child(h)

def tile_action(x, y):
    print("Memory: "+str(gc.mem_free()))
    global current_tilemap
    global dungeon_egress
    global current_dungeon_level
    global player_x
    global player_y
    global tiles_redraw
    global monster_redraw
    global deco_redraw
    global inventory_item_sel
    global inventory_ui_sel
    global current_msg
    global current_dungeon
    global control_mode
    if(inventory_ui_sel == 1):
        if(inventory_item_sel < len(player.inventory) and not current_tilemap.tile_has_item(x, y) and not current_tilemap.tile_solid(x, y)):
            item = player.inventory[inventory_item_sel]
            current_msg.text = "Dropped."
            current_tilemap.set_tile_data0(x, y, item.id)
            current_tilemap.set_tile_data1(x, y, 4)
            current_tilemap.set_tile_data2(x, y, item.data0)
            player.inventory.remove(item)
            player.wt -= Player.item_weights[item.id]
            if(player.held_item == item):
                player.held_item = None
                player.held_item_spr.opacity = 0.0
                player.remove_child(player.held_item_spr)
                player.held_item_spr = None
            deco_redraw = True
            monster_redraw = True
            return
        else:
            current_msg.text = "Can't drop here..."
            pass
    if(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1):
        if(player.held_item is not None and player.held_item.id == Player.item_ids["bomb"]):
            # Place bomb
            if(inventory_item_sel < len(player.inventory) and not current_tilemap.tile_occupied(x, y) and not current_tilemap.tile_solid(x, y)):
                player.held_item.data0 = 3
                item = player.held_item
                current_msg.text = "Placed."
                m = Monsters.Monster()
                m.set_monster(Monsters.monster_ids["litbomb"])
                m.stun = 3
                m.position = Vector2(x, y)
                current_tilemap.monster_list.append(m)

                player.inventory.remove(item)
                player.wt -= Player.item_weights[item.id]
                if(player.held_item == item):
                    player.held_item = None
                    player.held_item_spr.opacity = 0.0
                    player.remove_child(player.held_item_spr)
                    player.held_item_spr = None
                deco_redraw = True
                monster_redraw = True
                return
        Render.tile_animate_action(int(2 + selection_pos.x), int(2 + selection_pos.y), renderer_reload)
    if((current_tilemap.get_tile_data1(x, y) & (1 << 2)) == 0):
        if(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1 and current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["shopkeep1"]):
            # Start shopping
            print("Acted on shopkeeper")
            inventory_renderer.inv = current_tilemap.shopkeep_inv
            control_mode = mode_shop
            pass
        elif(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1 and current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["door_sheet"]):
            # Enter dungeon
            print("Dungeons: ")
            print(Generate.dungeons)
            if(len(dungeon_egress) == 0):
                # Enter dungeon
                current_dungeon = Generate.dungeons[(player_y+selection_pos.y)*Generate.MAX_WIDTH + (player_x+selection_pos.x)]
                current_msg.text = "Entered."
                dungeon_egress.append(Vector2(player_x, player_y))
                current_tilemap = current_dungeon.dungeon_levels[current_dungeon_level]
                player_x = current_tilemap.entryway.x
                player_y = current_tilemap.entryway.y
                current_dungeon_level += 1
            else:
                # Leave to overworld
                if(current_dungeon_level == 1):
                    current_tilemap = overworld_tiles
                player_x = dungeon_egress[len(dungeon_egress)-1].x
                player_y = dungeon_egress[len(dungeon_egress)-1].y
                dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
                current_dungeon_level = 0
                current_dungeon = None
        elif(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1 and current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["trapdoor_sheet"]):
            dungeon_egress.append(Vector2(player_x, player_y))
            current_msg.text = "Entered."
            current_tilemap = current_dungeon.dungeon_levels[current_dungeon_level]
            player_x = current_tilemap.entryway.x
            player_y = current_tilemap.entryway.y
            current_dungeon_level += 1
        elif(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1 and current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["ladder_sheet"]):
            current_tilemap = current_dungeon.dungeon_levels[current_dungeon_level-2]
            player_x = dungeon_egress[len(dungeon_egress)-1].x
            player_y = dungeon_egress[len(dungeon_egress)-1].y
            dungeon_egress.remove(dungeon_egress[len(dungeon_egress)-1])
            current_dungeon_level -= 1
        elif(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1 and current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["chest_sheet"]):
            left = urandom.randrange(2, 4)
            dx = -1
            dy = -1
            current_tilemap.set_tile_data0(x, y, 0)
            while(left > 0):
                pos = Generate.get_free_pos(current_tilemap, x-1, x+2, y-1, y+2)
                Generate.generate_item(current_tilemap, pos.x, pos.y)
                left -= 1
        elif(player.held_item is not None and selection_pos.x == 0 and selection_pos.y == 0):
            #print("Using item on self!")
            held_item = player.held_item.id
            if(held_item == Player.item_ids["blue_book"]):
                # Use spell on self
                if(player.mp < Player.spell_mana_cost[player.held_item.data0]):
                    current_msg.text = "Not enough mana!"
                else:
                    if(player.held_item.data0 == Player.spell_ids["novice_heal"]):
                        player.hp += 4
                    elif(player.held_item.data0 == Player.spell_ids["intermediate_heal"]):
                        player.hp += 6
                    elif(player.held_item.data0 == Player.spell_ids["advanced_heal"]):
                        player.hp += 10
                    player.mp -= Player.spell_mana_cost[player.held_item.data0]
                    if(player.hp > player.maxhp):
                        player.hp = player.maxhps
            elif(held_item == Player.item_ids["blue_pot"]):
                player.mp += 10
                if(player.mp > player.maxmp):
                    player.mp = player.maxmp
                player.inventory.remove(player.held_item)
                player.held_item = None
                player.held_item_spr.opacity = 0.0
                player.remove_child(player.held_item_spr)
                player.held_item_spr = None
            elif(Player.hp_values[held_item] is not None):
                # Consume item
                player.hp += Player.hp_values[held_item]
                if(player.hp > player.maxhp):
                    player.hp = player.maxhp
                player.inventory.remove(player.held_item)
                player.held_item = None
                player.held_item_spr.opacity = 0.0
                player.remove_child(player.held_item_spr)
                player.held_item_spr = None
        else:
            for m in current_tilemap.monster_list:
                if(player.held_item is not None and m.position.x == x and m.position.y == y):
                    #print("Attacking monster with hp " + str(m.hp))
                    held_item = player.held_item.id
                    dmg = Player.dmg_values[held_item]
                    stun = 0
                    if(held_item == Player.item_ids["blue_book"]):
                        if(player.mp < Player.spell_mana_cost[player.held_item.data0]):
                            current_msg.text = "Not enough mana!"
                        else:
                            spell = {i for i in Player.spell_ids if Player.spell_ids[i] == player.held_item.data0}
                            #print("using blue book spell ",spell)
                            if(player.held_item.data0 == Player.spell_ids["novice_confusion"]):
                                stun = 2
                            elif(player.held_item.data0 == Player.spell_ids["intermediate_confusion"]):
                                stun = 4
                            elif(player.held_item.data0 == Player.spell_ids["advanced_confusion"]):
                                stun = 5
                            elif(player.held_item.data0 == Player.spell_ids["lesser_mage_push"]):
                                stun = 1
                                #print("Lesser push on "+str(selection_pos.x)+", "+str(selection_pos.y))
                                if(selection_pos.x > 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x+1, m.position.y) and
                                    not current_tilemap.tile_occupied(m.position.x+1, m.position.y)):
                                        m.position.x += 1
                                elif(selection_pos.x < 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x-1, m.position.y) and
                                    not current_tilemap.tile_occupied(m.position.x-1, m.position.y)):
                                        m.position.x -= 1
                                elif(selection_pos.y > 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x, m.position.y+1) and
                                    not current_tilemap.tile_occupied(m.position.x, m.position.y+1)):
                                        m.position.y += 1
                                elif(selection_pos.y < 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x, m.position.y-1) and
                                    not current_tilemap.tile_occupied(m.position.x, m.position.y-1)):
                                        m.position.y -= 1
                            elif(player.held_item.data0 == Player.spell_ids["greater_mage_push"]):
                                stun = 2
                                if(selection_pos.x > 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x+1, m.position.y) and
                                    not current_tilemap.tile_occupied(m.position.x+1, m.position.y)):
                                        if(not current_tilemap.tile_solid(m.position.x+2, m.position.y) and
                                        not current_tilemap.tile_occupied(m.position.x+2, m.position.y)):
                                            m.position.x += 2
                                        else:
                                            m.position.x += 1
                                elif(selection_pos.x < 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x-1, m.position.y) and
                                    not current_tilemap.tile_occupied(m.position.x-1, m.position.y)):
                                        if(not current_tilemap.tile_solid(m.position.x-2, m.position.y) and
                                        not current_tilemap.tile_occupied(m.position.x-2, m.position.y)):
                                            m.position.x -= 2
                                        else:
                                            m.position.x -= 1
                                elif(selection_pos.y > 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x, m.position.y+1) and
                                    not current_tilemap.tile_occupied(m.position.x, m.position.y+1)):
                                        if(not current_tilemap.tile_solid(m.position.x, m.position.y+2) and
                                        not current_tilemap.tile_occupied(m.position.x, m.position.y+2)):
                                            m.position.y += 2
                                        else:
                                            m.position.y += 1
                                elif(selection_pos.y < 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                                    if(not current_tilemap.tile_solid(m.position.x, m.position.y-1) and
                                    not current_tilemap.tile_occupied(m.position.x, m.position.y-1)):
                                        if(not current_tilemap.tile_solid(m.position.x, m.position.y-2) and
                                        not current_tilemap.tile_occupied(m.position.x, m.position.y-2)):
                                            m.position.y -= 2
                                        else:
                                            m.position.y -= 1
                            player.mp -= Player.spell_mana_cost[player.held_item.data0]
                    elif(held_item == Player.item_ids["red_book"]):
                        if(player.mp < Player.spell_mana_cost[player.held_item.data0]):
                            current_msg.text = "Not enough mana!"
                        else:
                            spell = {i for i in Player.spell_ids if Player.spell_ids[i] == player.held_item.data0}
                            dmg = Player.spell_dmg_values[player.held_item.data0]
                            if(player.held_item.data0 == Player.spell_ids["novice_leech"] or
                            player.held_item.data0 == Player.spell_ids["intermediate_leech"] or
                            player.held_item.data0 == Player.spell_ids["advanced_leech"]):
                                player.hp += dmg
                            player.mp -= Player.spell_mana_cost[player.held_item.data0]
                    if(dmg is not None and m.id != Monsters.monster_ids["litbomb"]):
                        dmg += urandom.randrange(3)
                        m.hp -= dmg
                        current_msg.text = "Hit "+str(dmg) + "pts "
                        start_hitmarker(hitmarkers[0], selection_pos.x, selection_pos.y, dmg)
                    if(stun != 0 and m.id != Monsters.monster_ids["litbomb"]):
                        m.stun += stun
                        current_msg.text += "(+"+str(stun)+" stun)"
                    if(m.hp <= 0 and m.id != Monsters.monster_ids["litbomb"]):
                        gp = urandom.randrange(5, 12)
                        player.gp += gp
                        current_msg.text = "+"+str(gp)+" gp!"
                        current_tilemap.monster_list.remove(m)
            deco_redraw = True
            monster_redraw = True
            tiles_redraw = True
    elif(abs(selection_pos.x) <= 1 and abs(selection_pos.y) <= 1):
        # Item tile
        if(player.held_item is not None and player.held_item.id == Player.item_ids["blue_book"] and
        player.held_item.data0 == Player.spell_ids["lesser_mage_push"]):
            #Push item
            if(player.mp < Player.spell_mana_cost[player.held_item.data0]):
                current_msg.text = "Not enough mana!"
            else:
                item = current_tilemap.get_tile_data0(x, y)
                data0 = current_tilemap.get_tile_data2(x, y)
                if(selection_pos.x > 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                    if(not current_tilemap.tile_solid(x+1, y) and
                    not current_tilemap.tile_has_item(x+1, y)):
                        current_tilemap.set_tile_data0(x, y, 0)
                        current_tilemap.set_tile_data2(x, y, 0)
                        current_tilemap.set_deco_item(x, y, False)
                        current_tilemap.set_tile_data0(x+1, y, item)
                        current_tilemap.set_deco_item(x+1, y, True)
                        current_tilemap.set_tile_data2(x+1, y, data0)
                elif(selection_pos.x < 0 and abs(selection_pos.x) > abs(selection_pos.y)):
                    if(not current_tilemap.tile_solid(x-1, y) and
                    not current_tilemap.tile_has_item(x-1, y)):
                        current_tilemap.set_tile_data0(x, y, 0)
                        current_tilemap.set_tile_data2(x, y, 0)
                        current_tilemap.set_deco_item(x, y, False)
                        current_tilemap.set_tile_data0(x-1, y, item)
                        current_tilemap.set_deco_item(x-1, y, True)
                        current_tilemap.set_tile_data2(x-1, y, data0)
                elif(selection_pos.y > 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                    if(not current_tilemap.tile_solid(x, y+1) and
                    not current_tilemap.tile_has_item(x, y+1)):
                        current_tilemap.set_tile_data0(x, y, 0)
                        current_tilemap.set_tile_data2(x, y, 0)
                        current_tilemap.set_deco_item(x, y, False)
                        current_tilemap.set_tile_data0(x, y+1, item)
                        current_tilemap.set_deco_item(x, y+1, True)
                        current_tilemap.set_tile_data2(x, y+1, data0)
                elif(selection_pos.y < 0 and abs(selection_pos.x) <= abs(selection_pos.y)):
                    if(not current_tilemap.tile_solid(x, y-1) and
                    not current_tilemap.tile_has_item(x, y-1)):
                        current_tilemap.set_tile_data0(x, y, 0)
                        current_tilemap.set_tile_data2(x, y, 0)
                        current_tilemap.set_deco_item(x, y, False)
                        current_tilemap.set_tile_data0(x, y-1, item)
                        current_tilemap.set_deco_item(x, y-1, True)
                        current_tilemap.set_tile_data2(x, y-1, data0)
                player.mp -= Player.spell_mana_cost[player.held_item.data0]
        else:
            item = current_tilemap.get_tile_data0(x, y)
            data0 = current_tilemap.get_tile_data2(x, y)
            spell = list({i for i in Player.spell_ids if Player.spell_ids[i] == data0})
            if(item == Player.item_ids["red_book"] or item == Player.item_ids["blue_book"]):
                current_msg.text = spell[0]
            if(item == Player.item_ids["gold_coins"] or item == Player.item_ids["gold_bars"]):
                gold = urandom.randrange(5, 12)
                player.gp += gold
                current_tilemap.set_tile_data0(x, y, 0)
                current_tilemap.set_tile_data2(x, y, 0)
                current_tilemap.set_deco_item(x, y, False)
                current_msg.text = "+"+str(gold)+" gp!"
            elif(item >= Player.item_ids["rune1"] and item <= Player.item_ids["rune8"]):
                current_msg.text = "Found a rune fragment!"

                player.runes += 1
                if(player.runes == 1):
                    player.add_inv_item(Player.item_ids["rune_complete"], 0)
                current_tilemap.set_tile_data0(x, y, 0)
                current_tilemap.set_tile_data2(x, y, 0)
                current_tilemap.set_deco_item(x, y, False)
            elif(player.add_inv_item(item, data0)):
                current_tilemap.set_tile_data0(x, y, 0)
                current_tilemap.set_tile_data2(x, y, 0)
                current_tilemap.set_deco_item(x, y, False)
            else:
                current_msg.text = "Too heavy!"
                pass

turn_counter = 0

def action(direction):
    global player_x
    global player_y
    global selection_pos
    global monster_turn
    global current_msg
    current_msg.text = ""
    if(control_mode == mode_move):
        if (direction == 0):
            # Try to move rightward
            if(not current_tilemap.tile_solid(player_x+1, player_y)):
                for m in current_tilemap.monster_list:
                    if(m.position.x == player_x+1 and m.position.y == player_y):
                        return
                move_tiles_left() # Right tile is nonsolid
                monster_turn = True
        elif (direction == 1):
            # Try to move leftward
            if(not current_tilemap.tile_solid(player_x-1, player_y)):
                for m in current_tilemap.monster_list:
                    if(m.position.x == player_x-1 and m.position.y == player_y):
                        return
                move_tiles_right() # Left tile is nonsolid
                monster_turn = True
        elif (direction == 2):
             # Try to move downward
            if(not current_tilemap.tile_solid(player_x, player_y+1)):
                for m in current_tilemap.monster_list:
                    if(m.position.x == player_x and m.position.y == player_y+1):
                        return
                move_tiles_up() # Down tile is nonsolid
                monster_turn = True
        elif (direction == 3):
            # Try to move upward
            if(not current_tilemap.tile_solid(player_x, player_y-1)):
                for m in current_tilemap.monster_list:
                    if(m.position.x == player_x and m.position.y == player_y-1):
                        return
                move_tiles_down()
                monster_turn = True
    elif(control_mode == mode_action):
        if(player.held_item is None):
            range = 32
        else:
            range = 32*player.held_item.range
        if (direction == 0 and cursor_pos.x < range):
            selection_pos.x += 1
            cursor_pos.x += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 1 and cursor_pos.x > -range):
            selection_pos.x -= 1
            cursor_pos.x -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 2 and cursor_pos.y < range):
            selection_pos.y += 1
            cursor_pos.y += 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)
        elif (direction == 3 and cursor_pos.y > -range):
            selection_pos.y -= 1
            cursor_pos.y -= 32
            cursor.tween.start(cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN)

def set_cursor():
    cursor.texture = Tiles.action_indicator
    for m in current_tilemap.monster_list:
        if(player_x + selection_pos.x == m.position.x and player_y + selection_pos.y == m.position.y):
            cursor.texture = Tiles.attack_indicator

running = True

def show_death_screen(msg):
    print("STOPPING GAME")
    you_died = Text2DNode()
    you_died.font = Resources.roboto_font
    you_died.layer = 7
    you_died.position = Vector2(0, 0)
    you_died.text = "YOU DIED!"

    msgnode = Text2DNode()
    msgnode.font = Resources.roboto_font
    msgnode.layer = 7
    msgnode.position = Vector2(0, 10)
    msgnode.text = msg

    wherenode = Text2DNode()
    wherenode.font = Resources.roboto_font
    wherenode.layer = 7
    wherenode.position = Vector2(0, 20)
    wherenode.text = "in overworld" if len(dungeon_egress) == 0 else "on floor "+str(len(dungeon_egress))

    Render.cam.add_child(you_died)
    Render.cam.add_child(msgnode)
    Render.cam.add_child(wherenode)

    engine_io.release_all_buttons()

    while(not engine_io.A.is_just_pressed and not engine_io.B.is_just_pressed):
        engine.tick()

    you_died.mark_destroy_all()
    msgnode.mark_destroy_all()
    wherenode.mark_destroy_all()
    running = False
    machine.soft_reset()


class InventoryRenderer(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.inv = player.inventory
        self.sprlist = []
    def set_inv(inventory):
        self.inv = inventory

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

hp_text.layer = 7
mp_text.layer = 7

Render.cam.add_child(hp_text)
Render.cam.add_child(mp_text)

tiles_redraw = False
monster_redraw = True
deco_redraw = False

print("Entering game loop")

while running:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():

        action_dir = -1
        b_press = -1

        monster_turn = False
        tiles_redraw = False
        monster_redraw = False
        deco_redraw = False

        if engine_io.RIGHT.is_just_pressed:
            action_dir = 0
        elif engine_io.LEFT.is_just_pressed:
            action_dir = 1
        elif engine_io.DOWN.is_just_pressed:
            action_dir = 2
        elif engine_io.UP.is_just_pressed:
            action_dir = 3

        if engine_io.A.is_just_pressed:
            if(control_mode == mode_move):
                if(player.held_item is not None and player.held_item_spr.frame_count_x > 1):
                    # Animate hand item?
                    if(Player.item_animate[player.held_item.id] == 2):
                        player.held_item_spr.frame_current_x = 1
                control_mode = mode_action
                cursor.position = Vector2(0,0)
                cursor_pos = Vector2(0,0)
                selection_pos = Vector2(0,0)
                cursor.opacity = 1.0
            elif(control_mode == mode_action):
                if(player.held_item is not None):
                    player.held_item_spr.frame_current_x = 0
                control_mode = mode_move
                tile_action(int(selection_pos.x + player_x), int(selection_pos.y + player_y))
                cursor.opacity = 0.0
                monster_turn = True
            elif(control_mode == mode_inventory):
                if(inventory_ui_sel == 0):
                    #Equip item
                    if(inventory_item_sel < len(inventory_renderer.inv)):
                        if(inventory_renderer.inv[inventory_item_sel] != player.held_item):
                            player.set_held_item(inventory_renderer.inv[inventory_item_sel])
                        else:
                            player.held_item = None
                            player.held_item_spr.opacity = 0.0
                            player.remove_child(player.held_item_spr)
                            player.held_item_spr = None
                else:
                    #Drop item
                    if(inventory_item_sel < len(inventory_renderer.inv)):
                        control_mode = mode_action
                        cursor.position = Vector2(0,0)
                        cursor_pos = Vector2(0,0)
                        selection_pos = Vector2(0,0)
                        cursor.opacity = 1.0
            elif(control_mode == mode_shop):
                if(inventory_ui_sel == 0):
                    # Buy / sell item
                    if(inventory_renderer.inv == player.inventory):
                        # Sell item to shopkeeper
                        if(inventory_item_sel < len(player.inventory)):
                            item = player.inventory[inventory_item_sel]
                            current_tilemap.shopkeep_inv.append(item)
                            player.inventory.remove(item)
                            player.wt -= Player.item_weights[item.id]

                            if(item_id == Player.item_ids["red_book"] or item_id == Player.item_ids["blue_book"]):
                                player.gp += Player.spell_prices[inventory_renderer.inv[inventory_item_sel].data0][1]
                            else:
                                player.gp += math.floor(Player.item_prices[item_id][1] * Player.quality_price_modifiers[inventory_renderer.inv[inventory_item_sel].data0])

                            if(player.held_item == item):
                                player.held_item = None
                                player.held_item_spr.opacity = 0.0
                                player.remove_child(player.held_item_spr)
                                player.held_item_spr = None
                            inventory_item_sel -= 1
                            if(inventory_item_sel < 0):
                                inventory_item_sel = 0
                        pass
                    else:
                        # Buy item from shopkeeper
                        if(inventory_item_sel < len(current_tilemap.shopkeep_inv)):

                            price = 0
                            if(item_id == Player.item_ids["red_book"] or item_id == Player.item_ids["blue_book"]):
                                price = Player.spell_prices[inventory_renderer.inv[inventory_item_sel].data0][0]
                            else:
                                price = Player.item_prices[item_id][0] * Player.quality_price_modifiers[inventory_renderer.inv[inventory_item_sel].data0]

                            if(player.gp >= price):
                                item = current_tilemap.shopkeep_inv[inventory_item_sel]
                                player.inventory.append(item)
                                current_tilemap.shopkeep_inv.remove(item)
                                player.wt += Player.item_weights[item.id]
                                player.gp -= price
                                inventory_item_sel -= 1
                                if(inventory_item_sel < 0):
                                    inventory_item_sel = 0
                            else:
                                current_msg.text = "Not enough!"


                else:
                    # Switch inventory
                    if(inventory_renderer.inv == player.inventory):
                        inventory_renderer.inv = current_tilemap.shopkeep_inv
                    else:
                        inventory_renderer.inv = player.inventory
                    inventory_item_sel = 0

        if engine_io.B.is_just_pressed:
            inventory_item_sel = 0
            inventory_ui_sel = 0
            if(control_mode == mode_move):
                inventory_renderer.inv = player.inventory
                control_mode = mode_inventory
                inventory_ui_sel = 0
                pass
            elif(control_mode == mode_action or control_mode == mode_inventory or control_mode == mode_shop):
                if(player.held_item is not None):
                    player.held_item_spr.frame_current_x = 0
                control_mode = mode_move
                cursor.opacity = 0.0

        if(action_dir >= 0):
            action(action_dir)

        if(monster_turn):
            update_monsters()
            turn_counter += 1
            if(turn_counter >= 4):
                turn_counter = 0
                player.mp += 2
                if(player.mp > Player.MAX_MP):
                    player.mp = Player.MAX_MP
        if(tiles_redraw):
            Render.load_renderer_tiles(current_tilemap, player_x-2, player_y-2)
        if(deco_redraw):
            Render.load_renderer_deco(current_tilemap, player_x-2, player_y-2)
        if(monster_redraw):
            Render.load_renderer_monsters(current_tilemap)

        look_ang = 0.0
        player.scale = Vector2(1.0, 1.0)
        if(control_mode == mode_action and (player.held_item is not None) and Player.item_animate[player.held_item.id] > 0):
            look_ang = math.atan2(-cursor.position.y, cursor.position.x)
        player.look_ang = look_ang

        set_cursor()

        hp_text.text = "HP: "+str(player.hp) + " / " + str(player.maxhp)
        mp_text.text = "MP: "+str(player.mp) + " / " + str(player.maxmp)
        for n in range(0, inventory_renderer.get_child_count()):
            inventory_renderer.get_child(n).opacity = 0.0
        inventory_renderer.sprlist.clear()
        inventory_renderer.mark_destroy_children()

        if(control_mode == mode_inventory or control_mode == mode_shop):
            if(action_dir == 0):
                inventory_item_sel += 1
                if(inventory_item_sel >= len(inventory_renderer.inv)):
                    inventory_item_sel = len(inventory_renderer.inv)-1
            elif(action_dir == 1):
                inventory_item_sel -= 1
                if(inventory_item_sel < 0):
                    inventory_item_sel = 0
            elif(action_dir == 2 or action_dir == 3):
                inventory_ui_sel = 1-inventory_ui_sel

            gp_text = Text2DNode()
            wt_text = Text2DNode()
            runes_text = Text2DNode()

            name_text = Text2DNode()

            name_text.text = ""
            name_text.position = Vector2(0, -56)
            name_text.layer = 7
            name_text.font = Resources.roboto_font

            gp_text.text = "GP: "+str(player.gp)
            gp_text.font = Resources.roboto_font
            gp_text.layer = 7
            price = 0
            if(inventory_item_sel < len(inventory_renderer.inv)):
                item_id = inventory_renderer.inv[inventory_item_sel].id
                if(item_id == Player.item_ids["red_book"] or item_id == Player.item_ids["blue_book"]):
                    spell = list({i for i in Player.spell_ids if Player.spell_ids[i] == inventory_renderer.inv[inventory_item_sel].data0})[0]
                    name_text.text += spell
                    price = Player.spell_prices[inventory_renderer.inv[inventory_item_sel].data0][0 if inventory_renderer.inv != player.inventory else 1]
                else:
                    name_text.text += list({i for i in Player.item_qualities if Player.item_qualities[i] == inventory_renderer.inv[inventory_item_sel].data0})[0]
                    name_text.text += " "
                    name_text.text += list({i for i in Player.item_ids if Player.item_ids[i] == item_id})[0]
                    price = Player.item_prices[item_id][0 if inventory_renderer.inv != player.inventory else 1]

            if(control_mode == mode_inventory):

                eqp = Text2DNode()
                drop = Text2DNode()
                if((inventory_item_sel < len(inventory_renderer.inv)) and inventory_renderer.inv[inventory_item_sel] == player.held_item):
                    if(inventory_ui_sel == 0):
                        eqp.text = ">unequip"
                        drop.text = "drop"
                    else:
                        eqp.text = "unequip"
                        drop.text = ">drop"
                else:
                    if(inventory_ui_sel == 0):
                        eqp.text = ">equip/use"
                        drop.text = "drop"
                    else:
                        eqp.text = "equip/use"
                        drop.text = ">drop"
                wt_text.text = "WT: "+str(player.wt)+"/"+str(player.maxwt)
                runes_text.text = "Runes: "+str(player.runes)


                wt_text.font = Resources.roboto_font
                eqp.font = Resources.roboto_font
                drop.font = Resources.roboto_font
                runes_text.font = Resources.roboto_font

                gp_text.position =  Vector2(32, -4)
                wt_text.position =  Vector2(32, 4)
                runes_text.position = Vector2(32, 12)

                eqp.position = Vector2(-32, -4)
                drop.position = Vector2(-32, 4)

                wt_text.layer = 7
                eqp.layer = 7
                drop.layer = 7
                runes_text.layer = 7

                inventory_renderer.add_child(wt_text)
                inventory_renderer.add_child(eqp)
                inventory_renderer.add_child(drop)
                inventory_renderer.add_child(runes_text)
            else:
                # Render shop ui
                buysell = Text2DNode()
                view = Text2DNode()
                pricetext = Text2DNode()

                if(inventory_renderer.inv == current_tilemap.shopkeep_inv):
                    if(inventory_ui_sel == 0):
                        buysell.text = ">buy"
                        view.text = "view inv"
                    else:
                        buysell.text = "buy"
                        view.text = ">view inv"
                else:
                    if(inventory_ui_sel == 0):
                        buysell.text = ">sell"
                        view.text = "view shop"
                    else:
                        buysell.text = "sell"
                        view.text = ">view shop"
                pricetext.text = "price: " + str(price) + " GP"
                buysell.layer = 7
                view.layer = 7
                pricetext.layer = 7
                buysell.font = Resources.roboto_font
                view.font = Resources.roboto_font
                pricetext.font = Resources.roboto_font
                buysell.position = Vector2(-32, -4)
                view.position = Vector2(-32, 4)
                pricetext.position = Vector2(32, -4)
                gp_text.position =  Vector2(32, 4)
                inventory_renderer.add_child(buysell)
                inventory_renderer.add_child(view)
                inventory_renderer.add_child(pricetext)
            inventory_renderer.add_child(name_text)
            inventory_renderer.add_child(gp_text)

            item_spr_offset = Vector2(-inventory_item_sel*32, -32)
            for i in inventory_renderer.inv:

                frame_spr = Sprite2DNode()
                frame_spr.texture = Resources.frame_texture
                frame_spr.position.x = item_spr_offset.x
                frame_spr.position.y = item_spr_offset.y
                frame_spr.playing = False
                frame_spr.transparent_color  = Color(0x07e0)
                frame_spr.layer = 6
                frame_spr.frame_count_x = 1
                frame_spr.frame_current_x = 0

                item_spr = Sprite2DNode()
                item_spr.texture = i.texture
                item_spr.position.x = item_spr_offset.x
                item_spr.position.y = item_spr_offset.y
                item_spr.playing = False
                item_spr.transparent_color  = Color(0x07e0)
                item_spr.layer = 7
                item_spr.frame_count_x = i.frame_count
                item_spr.frame_current_x = 0
                item_spr_offset.x += 32
                inventory_renderer.sprlist.append(frame_spr)
                inventory_renderer.sprlist.append(item_spr)
                inventory_renderer.add_child(frame_spr)
                inventory_renderer.add_child(item_spr)
