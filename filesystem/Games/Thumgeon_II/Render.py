import math
import urandom

#from Thumgeon_II import renderer_reload

from engine_nodes import Sprite2DNode, CameraNode, Text2DNode
from engine_math import Vector2, Vector3
from engine_animation import Tween
from engine_draw import Color
import Tiles
import Player
import Monsters
import Resources

from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN, EASE_ELAST_OUT

class DrawTile(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = Tiles.grass1
        self.frame_count_x = 1
        self.width = 32
        self.height = 32
        #self.scale = Vector2(urandom.random(), urandom.random())
        self.id = 0
        self.monster = None
        self.layer = 1
        self.tween = Tween()
        self.deco = None

    @micropython.native
    def add_deco(self, deco_id, layer = 7):
        if deco_id != Tiles.deco_ids["none"]:
            self.deco = Sprite2DNode()
            self.deco.texture = Tiles.deco_textures[deco_id]
            self.deco.layer = layer
            self.deco.scale = Vector2(1, 1)
            self.deco.frame_count_x = Tiles.deco_frame_count[deco_id]
            self.deco.frame_current_x = 0
            self.deco.transparent_color = Color(0x07e0)
            self.deco.playing = False
            #self.deco.fps = 1.0
            #self.deco.rotation = 0.0
            #deco_sprite.opacity = 0.7
            self.add_child(self.deco)

    @micropython.native
    def add_item(self, item_id):
        if item_id != Player.item_ids["none"]:
            self.deco = Sprite2DNode()
            self.deco.texture = Player.item_textures[item_id]
            self.deco.layer = 5
            self.deco.scale = Vector2(1, 1)
            self.deco.frame_count_x = Player.item_frame_count[item_id]
            self.deco.frame_current_x = 0
            self.deco.transparent_color = Color(0x07e0)
            self.deco.playing = False
            #self.deco.fps = 1.0
            #self.deco.rotation = 0.0
            #deco_sprite.opacity = 0.7
            self.add_child(self.deco)

    @micropython.native
    def set_monster(self, m):
        if m.id >= 0:
            self.monster = m
            self.add_child(m)

    @micropython.native
    def reset_monster(self):
        if(self.monster is not None):
            #self.remove_child(self.monster)
            for n in range(self.get_child_count()):
                self.get_child(n).opacity = 0.0
            self.mark_destroy_children()
            self.monster.opacity = 0.0
            self.monster.mark_destroy()
            self.monster = None

    @micropython.native
    def reset_deco(self):
        if(self.deco is not None):
            self.deco.opacity = 0.0
        if(self.monster is not None):
            self.monster.opacity = 0.0
        #self.opacity = 0.0
        for n in range(self.get_child_count()):
            self.get_child(n).opacity = 0.0
        self.mark_destroy_children()
        self.deco = None
        self.monster = None

renderer_tiles = [None] * 6 * 6

for x in range(0, 6):
    for y in range(0, 6):
        renderer_tiles[y*6+x] = DrawTile()
        renderer_tiles[y*6+x].position = Vector2(32*(x), 32*(y))

cam = CameraNode()
camera_tween = Tween()
camera_offset = Vector2(64, 64)

anim_tween = Tween()

anim_snap = 600

renderer_x = 0
renderer_y = 0

@micropython.native
def tile_animate_action(x, y, after = None, frame=None):
    print(renderer_tiles[y*6+x].deco)
    if renderer_tiles[y*6+x].deco is not None:
        #renderer_tiles[y*6+x].deco.playing = True
        if(frame is None):
            if renderer_tiles[y*6+x].deco.frame_count_x > 1:
                renderer_tiles[y*6+x].deco.frame_current_x = 1
        else:
            renderer_tiles[y*6+x].deco.frame_current_x = frame
        start = ((urandom.random() + 0.3)) * math.pi/8
        if(urandom.random() < 0.5):
            start = -start
        if(renderer_tiles[y*6+x].deco is not None):
            anim_tween.start(renderer_tiles[y*6+x].deco, "rotation", start, 0.0, anim_snap, 1.0, ONE_SHOT, EASE_ELAST_OUT)
        anim_tween.after = after

#@micropython.native
def load_renderer_monsters(tilemap, offset = Vector2(0,0)):
    #for y in range(0, 6):
    #    for x in range(0, 6):
    #        if(renderer_tiles[y*6+x].monster is not -1):
    #            print("Removing " + str(renderer_tiles[y*6+x].monster))
    #            renderer_tiles[y*6+x].remove_child(renderer_tiles[y*6+x].monster)
    #            renderer_tiles[y*6+x].monster = None
    for m in tilemap.monster_list:
        mx = int(m.position.x - renderer_x + offset.x)
        my = int(m.position.y - renderer_y + offset.y)
        if(mx >= 0 and mx < 6 and my >= 0 and my < 6):
            m_sprite = Monsters.Monster()
            m_sprite.texture = m.texture
            m_sprite.position = Vector2(0, 0)
            m_sprite.transparent_color = Color(0x07e0)
            m_sprite.layer = 5
            m_sprite.opacity = 1.0
            m_sprite.hp = 10
            m_sprite.frame_count_x = m.frame_count_x
            m_sprite.frame_current_x = m.frame_current_x

            textnode = Text2DNode()
            textnode.text = str(m.hp)
            textnode.font = Resources.roboto_font
            textnode.opacity = 1.0
            textnode.layer = 7
            textnode.position.y = -16
            # for n in range(m_sprite.get_child_count()):
            #     m_sprite.get_child(n).opacity = 0.0
            # m_sprite.mark_destroy_children()
            m_sprite.add_child(textnode)
            m_sprite.textnote = textnode
            #m_sprite.tick = m.tick
            #print("Loading monster at "+str(mx)+", "+str(my))
            renderer_tiles[my*6+mx].set_monster(m_sprite)

#@micropython.native
def load_renderer_deco(tilemap, cx, cy):
    for y in range(0, 6):
        for x in range(0, 6):
            renderer_tiles[y*6+x].reset_monster()
            renderer_tiles[y*6+x].reset_deco()
            if((tilemap.get_tile_data1(int(cx+x), int(cy+y)) & (1 << 2)) != 0):
                renderer_tiles[y*6+x].add_item(tilemap.get_tile_data0(int(cx+x), int(cy+y)))
            else:
                renderer_tiles[y*6+x].add_deco(tilemap.get_tile_data0(int(cx+x), int(cy+y)), 4 if ((tilemap.get_tile_data1(int(cx+x), int(cy+y)) & 0x2) != 0) else 6)

#@micropython.native
def load_renderer_tiles(tilemap, cx, cy):
    cam.position = Vector3(64,64,1)
    for y in range(0, 6):
        for x in range(0, 6):
            #renderer_tiles[y*6+x].reset_deco()
            renderer_tiles[y*6+x].id = tilemap.get_tile_id(int(cx+x), int(cy+y))
            renderer_tiles[y*6+x].texture = Tiles.tile_textures[renderer_tiles[y*6+x].id]
            renderer_tiles[y*6+x].layer = 1
            #if((tilemap.get_tile_data1(int(cx+x), int(cy+y)) & (1 << 2)) != 0):
            #    renderer_tiles[y*6+x].add_item(tilemap.get_tile_data0(int(cx+x), int(cy+y)))
            #else:
            #    renderer_tiles[y*6+x].add_deco(tilemap.get_tile_data0(int(cx+x), int(cy+y)), 4 if ((tilemap.get_tile_data1(int(cx+x), int(cy+y)) & 0x2) != 0) else 6)
    #load_renderer_monsters(Vector2(0,0))
