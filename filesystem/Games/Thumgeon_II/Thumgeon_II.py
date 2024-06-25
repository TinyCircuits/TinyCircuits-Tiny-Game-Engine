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
from engine_animation import (
    Tween,
    Delay,
    ONE_SHOT,
    LOOP,
    PING_PONG,
    EASE_ELAST_OUT,
    EASE_ELAST_IN_OUT,
    EASE_BOUNCE_IN_OUT,
    EASE_SINE_IN,
    EASE_QUAD_IN,
)
import math
import os
import gc
import urandom
import utime

engine_physics.set_gravity(0.0, 0.0)
engine.set_fps_limit(50)
# engine.disable_fps_limit()
machine.freq(180000000)

os.chdir("/Games/Thumgeon_II")
roboto_font = FontResource("9pt-roboto-font.bmp")

import Tiles
import Render
import Generate
import Player

print(os.getcwd())

player = Player.Player()


class CursorSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Tiles.action_indicator
        self.frame_count_x = 1
        self.frame_current_x = 0
        self.playing = False
        self.set_layer(6)
        self.opacity = 0.0
        self.transparent_color = Color(0x07E0)
        self.width = 32
        self.height = 32
        self.tween = Tween()
        Render.cam.add_child(self)


cursor = CursorSprite()
cursor_pos = Vector2(0, 0)

mode_move = 0
mode_action = 1

control_mode = mode_move

tween_snap = 80  # Controls camera tween speed
indicator_snap = 50  # Controls indicator tween speed

camera_z = 0.0


def draw_minimap():
    # Very slow to generate
    pass


overworld_tiles = Tiles.Tilemap(48, 48)

urandom.seed()
for y in range(0, overworld_tiles.HEIGHT):
    for x in range(0, overworld_tiles.WIDTH):
        overworld_tiles.tiles[(y * overworld_tiles.WIDTH + x) * 3] = 255

overworld_tiles.border_tile = Tiles.tile_ids["water1"]
Generate.generate_tiles(overworld_tiles)
# Generate.generate_water(overworld_tiles)
Generate.generate_deco(overworld_tiles)

dungeon_tiles = Tiles.Tilemap(urandom.randrange(5, 12), urandom.randrange(5, 12))

dungeon_egress = None

Generate.generate_empty_dungeon(dungeon_tiles)
Generate.generate_dungeon_level(dungeon_tiles)

current_tilemap = overworld_tiles

dungeon_levels = [None] * 5

for i in range(5):
    dungeon_levels[i] = Tiles.Tilemap(urandom.randrange(5, 12), urandom.randrange(5, 12))
    Generate.generate_empty_dungeon(dungeon_levels[i])
    Generate.generate_dungeon_level(dungeon_levels[i])

# current_tilemap = dungeon_tiles


def reset_camera_right(self):
    Render.renderer_x -= 1


def reset_camera_left(self):
    Render.renderer_x += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)


def reset_camera_down(self):
    Render.renderer_y += 1
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)


def reset_camera_up(self):
    Render.renderer_y -= 1


def move_tiles_right():
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x - 1, Render.renderer_y)
    Render.camera_tween.start(
        Render.cam,
        "position",
        Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0),
        Vector3(Render.camera_offset.x, Render.camera_offset.y, 0),
        tween_snap,
        1.0,
        ONE_SHOT,
        EASE_QUAD_IN,
    )
    Render.camera_tween.after = reset_camera_right


def move_tiles_left():
    Render.camera_tween.start(
        Render.cam,
        "position",
        Vector3(Render.camera_offset.x, Render.camera_offset.y, 0),
        Vector3(32 + Render.camera_offset.x, Render.camera_offset.y, 0),
        tween_snap,
        1.0,
        ONE_SHOT,
        EASE_QUAD_IN,
    )
    Render.camera_tween.after = reset_camera_left


def move_tiles_down():
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y - 1)
    Render.camera_tween.start(
        Render.cam,
        "position",
        Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0),
        Vector3(Render.camera_offset.x, Render.camera_offset.y, 0),
        tween_snap,
        1.0,
        ONE_SHOT,
        EASE_QUAD_IN,
    )
    Render.camera_tween.after = reset_camera_up


def move_tiles_up():
    Render.camera_tween.start(
        Render.cam,
        "position",
        Vector3(Render.camera_offset.x, Render.camera_offset.y, 0),
        Vector3(Render.camera_offset.x, 32 + Render.camera_offset.y, 0),
        tween_snap,
        1.0,
        ONE_SHOT,
        EASE_QUAD_IN,
    )
    Render.camera_tween.after = reset_camera_down


Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
# gc.enable()
gc.threshold(1024 * 32)

selection_pos = Vector2(0, 0)


def renderer_reload(dummy):
    Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    # Render.anim_tween.restart()


def tile_action(x, y):
    global current_tilemap
    global dungeon_tiles
    global dungeon_egress
    print(str(2 + selection_pos.x) + ", " + str(2 + selection_pos.y))
    Render.tile_animate_action(int(2 + selection_pos.x), int(2 + selection_pos.y), renderer_reload)
    if (current_tilemap.get_tile_data1(x, y) & (1 << 2)) == 0:
        if current_tilemap.get_tile_data0(x, y) == Tiles.deco_ids["door_sheet"]:
            print("Acted on door!")
            if dungeon_egress is None:
                dungeon_egress = Vector2(Render.renderer_x, Render.renderer_y)
                current_tilemap = dungeon_tiles
                Render.renderer_x = 0
                Render.renderer_y = 0
            else:
                current_tilemap = overworld_tiles
                Render.renderer_x = dungeon_egress.x
                Render.renderer_y = dungeon_egress.y
                dungeon_egress = None
        else:
            print("Other action")
    else:
        # Item tile
        pass

    # Render.load_renderer_tiles(current_tilemap, Render.renderer_x, Render.renderer_y)
    # renderer_reload(None)


def action(direction):
    px = Render.renderer_x + 2
    py = Render.renderer_y + 2
    global selection_pos
    if control_mode == mode_move:
        if direction == 0:
            # Try to move rightward
            if (current_tilemap.get_tile_data1(int(px + 1), int(py)) & 0x1) == 0:
                move_tiles_left()  # Right tile is nonsolid
        elif direction == 1:
            # Try to move leftward
            if (current_tilemap.get_tile_data1(int(px - 1), int(py)) & 0x1) == 0:
                move_tiles_right()  # Left tile is nonsolid
        elif direction == 2:
            # Try to move downward
            if (current_tilemap.get_tile_data1(int(px), int(py + 1)) & 0x1) == 0:
                move_tiles_up()  # Down tile is nonsolid
        elif direction == 3:
            # Try to move upward
            if (current_tilemap.get_tile_data1(int(px), int(py - 1)) & 0x1) == 0:
                move_tiles_down()
    elif control_mode == mode_action:
        if direction == 0 and cursor_pos.x < 32:
            selection_pos.x += 1
            cursor_pos.x += 32
            cursor.tween.start(
                cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN
            )
        elif direction == 1 and cursor_pos.x > -32:
            selection_pos.x -= 1
            cursor_pos.x -= 32
            cursor.tween.start(
                cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN
            )
        elif direction == 2 and cursor_pos.y < 32:
            selection_pos.y += 1
            cursor_pos.y += 32
            cursor.tween.start(
                cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN
            )
        elif direction == 3 and cursor_pos.y > -32:
            selection_pos.y -= 1
            cursor_pos.y -= 32
            cursor.tween.start(
                cursor, "position", cursor.position, cursor_pos, indicator_snap, 1.0, ONE_SHOT, EASE_QUAD_IN
            )


def MessageSprite(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.text = "testing"
        self.set_layer(7)
        self.position = Vector2(0, 0)
        self.font = roboto_font
        self.opacity = 1.0
        Render.cam.add_child(self)


current_msg = MessageSprite()

while True:
    # Only execute code as fast as the engine ticks (due to FPS limit)
    if engine.tick():

        # print(urandom.random())
        # gc.collect()
        print(engine.get_running_fps())
        print(gc.mem_free())
        # print(gc.mem_free())
        # print(str(Render.renderer_x) + ", " + str(Render.renderer_y))
        # print(str(selection_pos.x) + ", " + str(selection_pos.y))
        action_dir = -1

        if engine_io.RIGHT.is_just_pressed:
            action_dir = 0
        elif engine_io.LEFT.is_just_pressed:
            action_dir = 1
        elif engine_io.DOWN.is_just_pressed:
            action_dir = 2
        elif engine_io.UP.is_just_pressed:
            action_dir = 3

        if engine_io.A.is_just_pressed:
            if control_mode == mode_move:
                control_mode = mode_action
                cursor.position = Vector2(0, 0)
                cursor_pos = Vector2(0, 0)
                selection_pos = Vector2(0, 0)
                cursor.opacity = 1.0
            elif control_mode == mode_action:
                tile_action(int(2 + selection_pos.x + Render.renderer_x), int(2 + selection_pos.y + Render.renderer_y))
                control_mode = mode_move
                cursor.opacity = 0.0

        if engine_io.B.is_just_pressed:
            if control_mode == mode_move:
                pass
            elif control_mode == mode_action:
                control_mode = mode_move
                cursor.opacity = 0.0

        if action_dir >= 0:
            action(action_dir)

        look_ang = 0.0
        player.scale = Vector2(1.0, 1.0)
        if control_mode == mode_action:
            look_ang = math.atan2(-cursor.position.y, cursor.position.x)
            print(look_ang)
            if look_ang > math.pi / 2 or look_ang < -math.pi / 2:
                player.scale.x = -1.0
        player.held_item_spr.rotation = look_ang
