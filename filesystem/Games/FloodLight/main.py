import engine_main

import engine
from engine_draw import Color
import engine_draw
from engine_nodes import Rectangle2DNode, CameraNode, EmptyNode, Sprite2DNode, Text2DNode
from engine_animation import Tween, ONE_SHOT, EASE_ELAST_IN_OUT
from engine_math import Vector2, Vector3
from engine_resources import TextureResource, FontResource
import engine_io

engine.fps_limit(60)

font = FontResource("outrunner_outline.bmp")

flood_node = Rectangle2DNode(Vector2(-1,-1),128,128,engine_draw.white,1.0,False)
flood_preview_node = Rectangle2DNode(Vector2(-1,303),128,16,engine_draw.white,1.0,False)

class ContainerNode(EmptyNode):
    def __init__(self, position=Vector2(0,0)):
        super().__init__(self)
        self.position = position    

light_root_node = ContainerNode(Vector2(0,0))
light_textures = []
light_nodes = []
light_coords = []
light_index = 0
x = 0
light_coords.append(Vector2(x,0))
x -= 128 + 1
for file in [
        "light80.bmp",
        "light60.bmp",
        "light40.bmp",
        "light20.bmp",
        ]:
    tex = TextureResource(file)
    light_textures.append(tex)
    light_coords.append(Vector2(x,0))
    for row in range(4):
        for col in range(4):
            cx = 32*col + x - 48
            cy = 32*row - 48
            node = Sprite2DNode(Vector2(cx,cy),tex,engine_draw.white)
            light_nodes.append(node)
            light_root_node.add_child(node)
    x -= 128 + 1
node = Rectangle2DNode(Vector2(x,0),128,128,engine_draw.black,1,False)
light_nodes.append(node)
light_root_node.add_child(node)
light_coords.append(Vector2(x,0))
x -= 128 + 1

cursor_tex = TextureResource("cursor.bmp")
palette_root_node = ContainerNode(Vector2(0,256))
palette_textures = []
palette_nodes = []
palette_title_nodes = []
palette_cursor_nodes = []
palette_index = 0
x = 0
for file, title in [
        ("hue.bmp","HUE"),
        ("temp.bmp","TEMPERATURE"),
        ]:
    tex = TextureResource(file)
    palette_textures.append(tex)
    node = Sprite2DNode(Vector2(x,0),tex)
    palette_nodes.append(node)
    palette_root_node.add_child(node)
    title_node = Text2DNode(Vector2(x,-48),font,title)
    palette_root_node.add_child(title_node)
    palette_title_nodes.append(title_node)
    cursor_node = Sprite2DNode(Vector2(0,0),cursor_tex,engine_draw.black)
    cursor_node.opacity = 0.75
    palette_cursor_nodes.append(cursor_node)
    node.add_child(cursor_node)
    x += 256

cam = CameraNode()

cam_tween = Tween()
light_tween = Tween()
palette_tween = Tween()

while True:
    
    update_tween = True
    cam_tween.start(cam, "position", cam.position, Vector3(0,0,0), 500, ONE_SHOT, EASE_ELAST_IN_OUT)
    while True: # Flood Light Mode
        if engine.tick():
            if engine_io.A.is_just_pressed or engine_io.B.is_just_pressed:
                break # Switch modes
            
            # Brightness
            if (engine_io.RB.is_just_pressed or engine_io.RIGHT.is_just_pressed) and light_index > 0:
                light_index -= 1
                update_tween = True
            if (engine_io.LB.is_just_pressed or engine_io.LEFT.is_just_pressed) and light_index < len(light_coords)-1:
                light_index += 1
                update_tween = True
            if update_tween:
                update_tween = False
                pos = light_coords[light_index]
                light_tween.start(light_root_node, "position", light_root_node.position, Vector2(-pos.x-1,-pos.y-1), 250, ONE_SHOT, EASE_ELAST_IN_OUT)
            
            
    update_tween = True
    update_color = True
    cam_tween.start(cam, "position", cam.position, Vector3(0,256,0), 500, ONE_SHOT, EASE_ELAST_IN_OUT)
    while True: # Color Pick Mode
        if engine.tick():
            if engine_io.A.is_just_pressed or engine_io.B.is_just_pressed:
                break # Switch modes
            
            # Switch Palette
            if engine_io.LB.is_just_pressed and palette_index > 0:
                palette_index -= 1
                update_tween = True
            if engine_io.RB.is_just_pressed and palette_index < len(palette_nodes)-1:
                palette_index += 1
                update_tween = True
            if update_tween:
                update_tween = False
                pos = palette_nodes[palette_index].position
                palette_tween.start(palette_root_node, "position", palette_root_node.position, Vector2(-pos.x-1,256-pos.y-1), 250, ONE_SHOT, EASE_ELAST_IN_OUT)
                update_color = True

            # Cursor
            cursor = palette_cursor_nodes[palette_index]
            if engine_io.LEFT.is_pressed and cursor.position.x > -63:
                cursor.position.x -= 1
                update_color = True
            if engine_io.RIGHT.is_pressed and cursor.position.x < 64:
                cursor.position.x += 1
                update_color = True
            if engine_io.UP.is_pressed and cursor.position.y > -31:
                cursor.position.y -= 1
                update_color = True
            if engine_io.DOWN.is_pressed and cursor.position.y < 32:
                cursor.position.y += 1
                update_color = True
            
            if update_color:
                update_color = False
                tex_data = palette_textures[palette_index].data
                coord_x = int(cursor.position.x)+63
                coord_y = int(cursor.position.y)+31
                i = (coord_y*128 + coord_x) * 2
                color = (tex_data[i+1] << 8) | tex_data[i]
                flood_node.color = color
                flood_preview_node.color = color