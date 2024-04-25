import engine_main

import engine
import engine_draw
import engine_input
import time
from engine_math import Vector2
from engine_nodes import Sprite2DNode, CameraNode, GUIBitmapButton2DNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT

import math


engine.set_fps_limit(60)

font = FontResource("outrunner_outline.bmp")
launcher_tile_texture = TextureResource("launcher-tile.bmp")
launcher_tile_mark_texture = TextureResource("launcher-tile-mark.bmp")


class BatteryIndicator(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.text_node = Text2DNode(text="0%", font=font)
        self.position.x = -40
        self.position.y = -56
        self.add_child(self.text_node)
    
    def tick(self):
        self.text_node.text = str(round(engine.battery_level() * 100.0, 1)) + "%"


battery = BatteryIndicator()

tiles = []
last_focused_tile = None
rumble_intensity = 0.0

def after_rumble(tween):
    rumble_intensity = 0.0
    engine_input.rumble(rumble_intensity)

class LauncherTile(GUIBitmapButton2DNode):
    def __init__(self):
        super().__init__(self)

        self.font = font
        self.text = ""
        self.bitmap = launcher_tile_texture
        self.transparent_color = engine_draw.black
        self.scale.x = 0.35
        self.scale.y = 0.35

        self.title_text_node = Text2DNode(text="Unknown", font=font, position=Vector2(0, -28), opacity=0.0, letter_spacing=1.0)

        self.mark_sprite = Sprite2DNode(texture=launcher_tile_mark_texture)
        self.mark_sprite.scale.x = 0.35
        self.mark_sprite.scale.y = 0.35

        self.add_child(self.mark_sprite)
        self.add_child(self.title_text_node)

        self.t = 0.0
        self.set_opacity(0.65)

        tiles.append(self)

        self.tween = Tween()
        self.rumble_tween = Tween()
        self.rumble_tween.after = after_rumble
    
    def on_focused(self):
        self.position.y = math.sin(self.t)
        self.t += 0.05

    def on_just_focused(self):
        global last_focused_tile
        self.set_opacity(1.0)
        self.set_scale(0.4)
        self.title_text_node.opacity = 1.0

        if last_focused_tile != None:
            delta = 0
            if self.position.x > last_focused_tile.position.x:
                delta = -45
            else:
                delta = 45
            
            for tile in tiles:
                tile.tween.stop()
                tile.tween.start(tile, "position", tile.position, Vector2(tile.position.x+delta, 0), 250.0, 1.0, ONE_SHOT, EASE_BACK_OUT)

        last_focused_tile = self
    
    def on_just_unfocused(self):
        self.set_opacity(0.65)
        self.set_scale(0.3)
        self.position.y = 0
        self.title_text_node.opacity = 0.0
    
    def on_just_pressed(self):
        engine_input.rumble(0.2)
        time.sleep(0.1)
        engine_input.rumble(0.0)

    def set_opacity(self, opacity):
        self.opacity = opacity
        self.mark_sprite.opacity = opacity
    
    def set_scale(self, scale):
        self.scale.x = scale
        self.scale.y = scale
        self.mark_sprite.scale.x = scale
        self.mark_sprite.scale.y = scale


for i in range(5):
    tile = LauncherTile()
    tile.position.x = (i * 45)
    tile.set_scale(0.3)

camera = CameraNode()

engine_input.toggle_gui_focus()
engine_input.gui_toggle_button = None

while True:
    engine.tick()