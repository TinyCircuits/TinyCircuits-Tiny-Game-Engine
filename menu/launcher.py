import engine_main

import engine
import engine_draw
import engine_input
from engine_nodes import Sprite2DNode, CameraNode, GUIBitmapButton2DNode, Text2DNode
from engine_resources import TextureResource, FontResource

import math


engine.set_fps_limit(60)

font = FontResource("9pt-roboto-font.bmp")
launcher_tile_texture = TextureResource("launcher-tile.bmp")
launcher_tile_mark_texture = TextureResource("launcher-tile-mark.bmp")


# class BatteryIndicator(Sprite2DNode):
#     def __init__(self):
#         super().__init__(self)
#         self.text_node = Text2DNode(text="0%", font=font)
#         self.position.y = -32
#         self.add_child(self.text_node)
    
#     def tick(self):
#         self.text_node.text = str(engine.battery_level() * 100.0) + "%"


# battery = BatteryIndicator()


class LauncherTile(GUIBitmapButton2DNode):
    def __init__(self):
        super().__init__(self)

        self.font = font
        self.text = ""
        self.bitmap = launcher_tile_texture
        self.transparent_color = engine_draw.black
        self.scale.x = 0.35
        self.scale.y = 0.35

        self.mark_sprite = Sprite2DNode(texture=launcher_tile_mark_texture)
        self.mark_sprite.scale.x = 0.35
        self.mark_sprite.scale.y = 0.35

        self.add_child(self.mark_sprite)

        self.t = 0.0
    
    def on_focused(self):
        self.position.y = math.sin(self.t)
        self.t += 0.05

    def on_just_focused(self):
        print("Focused", self.text)
        self.set_opacity(1.0)
    
    def on_just_unfocused(self):
        print("Unfocused", self.text)
        self.set_opacity(0.65)
        self.position.y = 0

    
    def set_opacity(self, opacity):
        self.opacity = opacity
        self.mark_sprite.opacity = opacity
    
    def set_scale(self, scale):
        self.scale.x = scale
        self.scale.y = scale
        self.mark_sprite.scale.x = scale
        self.mark_sprite.scale.y = scale


tile0 = LauncherTile()
tile0.selected = True

tile1 = LauncherTile()
tile1.position.x = -42
tile1.set_scale(0.25)
tile1.set_opacity(0.65)

tile2 = LauncherTile()
tile2.position.x = 42
tile2.set_scale(0.25)
tile2.set_opacity(0.65)

camera = CameraNode()

engine_input.toggle_gui_focus()
engine_input.gui_toggle_button = None

while True:
    engine.tick()