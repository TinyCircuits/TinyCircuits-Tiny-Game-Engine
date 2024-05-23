import math

from engine_nodes import Sprite2DNode, CameraNode
from engine_math import Vector2, Vector3
from engine_animation import Tween
from engine_draw import Color
import Tiles

from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN

class DrawTile(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Tiles.grass1
        self.frame_count_x = 1
        self.width = 32
        self.height = 32
        #self.scale = Vector2(urandom.random(), urandom.random())
        self.id = 0
        self.set_layer(1)
        self.tween = Tween()
        self.deco = None
    
    @micropython.native
    def add_deco(self, deco_id, layer = 7):
        if deco_id != Tiles.deco_ids["none"]:
            deco_sprite = Sprite2DNode()
            deco_sprite.texture = Tiles.deco_textures[deco_id]
            deco_sprite.set_layer(layer)
            deco_sprite.scale = Vector2(1, 1)
            deco_sprite.frame_count_x = Tiles.deco_frame_count[deco_id]
            deco_sprite.transparent_color = Color(0x07e0)
            deco_sprite.playing = False
            #deco_sprite.rotation = 0.0
            #deco_sprite.opacity = 0.7
            self.add_child(deco_sprite)
            self.deco = deco_sprite
            
    @micropython.native
    def reset_deco(self):
        self.deco = None
        self.destroy_children()

renderer_tiles = [None] * 6 * 6

for x in range(0, 6):
    for y in range(0, 6):
        renderer_tiles[y*6+x] = DrawTile()
        renderer_tiles[y*6+x].position = Vector2(32*(x), 32*(y))
        
cam = CameraNode()
camera_tween = Tween()
camera_offset = Vector2(64, 64)

anim_snap = 2000

renderer_x = 0
renderer_y = 0

def tile_animate_action(x, y, after = None):
    print(renderer_tiles[y*6+x].deco)
    if renderer_tiles[y*6+x].deco is not None:
        renderer_tiles[y*6+x].tween.start(renderer_tiles[y*6+x].deco.rotation, "", 0.0, 2*math.pi, anim_snap, 1.0, ONE_SHOT, EASE_ELAST_IN_OUT)
        renderer_tiles[y*6+x].tween.after = after
        
#@micropython.native
def load_renderer_tiles(tilemap, cx, cy):
    cam.position = Vector3(64,64,1)
    for y in range(0, 6):
        for x in range(0, 6):
            renderer_tiles[y*6+x].reset_deco()
            renderer_tiles[y*6+x].id = tilemap.get_tile_id(int(cx+x), int(cy+y))
            renderer_tiles[y*6+x].texture = Tiles.tile_textures[renderer_tiles[y*6+x].id]
            renderer_tiles[y*6+x].set_layer(1)
            renderer_tiles[y*6+x].add_deco(tilemap.get_tile_data0(int(cx+x), int(cy+y)), 5 if ((tilemap.get_tile_data1(int(cx+x), int(cy+y)) & 0x2) != 0) else 7)