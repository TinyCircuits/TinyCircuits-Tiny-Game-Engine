import engine
import engine_draw
import engine_input
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource

import math


engine.set_fps_limit(60)


launcher_tile_texture = TextureResource("launcher-tile.bmp")
launcher_tile_mark_texture = TextureResource("launcher-tile-mark.bmp")

class LauncherTile(Sprite2DNode):
    def __init__(self):
        super().__init__(self)

        self.texture = launcher_tile_texture
        self.transparent_color = engine_draw.black
        self.scale.x = 0.35
        self.scale.y = 0.35

        self.mark_sprite = Sprite2DNode(texture=launcher_tile_mark_texture)
        self.mark_sprite.scale.x = 0.35
        self.mark_sprite.scale.y = 0.35

        self.add_child(self.mark_sprite)

        self.selected = False
        self.t = 0.0
    
    def tick(self):
        if self.selected:
            self.position.y = math.sin(self.t)
            self.t += 0.05
        else:
            self.position.y = 0.0
    
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

while True:

    engine.tick()