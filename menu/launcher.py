import engine_main

import engine
import engine_draw
import engine_input
import time
from engine_math import Vector2
from engine_nodes import Sprite2DNode, CameraNode, GUIBitmapButton2DNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT
import os
import array

import math


game_path_to_execute = None


engine.set_fps_limit(60)

font = FontResource("outrunner_outline.bmp")
launcher_tile_texture = TextureResource("launcher-tile.bmp")
launcher_tile_mark_texture = TextureResource("launcher-tile-mark.bmp")


class BatteryIndicator(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.text_node = Text2DNode(text="0%", font=font)
        self.position.x = -40
        self.position.y = -58
        self.add_child(self.text_node)
    
    def tick(self, dt):
        self.text_node.text = str(round(engine.battery_level() * 100.0, 1)) + "%"


battery = BatteryIndicator()

tiles = []
last_focused_tile = None
rumble_intensity = 0.0

def after_rumble(tween):
    rumble_intensity = 0.0
    engine_input.rumble(rumble_intensity)


class GameInfo():
    def __init__(self):
        self.game_name = None
        self.game_main_path = None
        self.game_icon_path = None


class LauncherTile(GUIBitmapButton2DNode):
    def __init__(self):
        super().__init__(self)

        self.font = font
        self.text = ""
        self.bitmap = launcher_tile_texture
        self.transparent_color = engine_draw.white
        self.scale.x = 0.35
        self.scale.y = 0.35
        self.set_layer(2)

        self.title_text_node = Text2DNode(text="Unknown", font=font, position=Vector2(0, -28), opacity=0.0, letter_spacing=1.0)

        self.icon_sprite = Sprite2DNode(texture=launcher_tile_mark_texture)
        self.icon_sprite.scale.x = 0.35
        self.icon_sprite.scale.y = 0.35
        self.icon_sprite.set_layer(0)

        self.add_child(self.icon_sprite)
        self.add_child(self.title_text_node)

        self.t = 0.0
        self.set_opacity(0.65)

        tiles.append(self)

        self.tween = Tween()
        self.rumble_tween = Tween()
        self.rumble_tween.after = after_rumble

        self.game_info = None

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

        if self.game_info != None and self.game_info.game_main_path != None:
            global game_path_to_execute
            game_path_to_execute = self.game_info.game_main_path
            engine.end()
            
    def set_opacity(self, opacity):
        self.opacity = opacity
        self.icon_sprite.opacity = opacity
    
    def set_scale(self, scale):
        self.scale.x = scale
        self.scale.y = scale
        self.icon_sprite.scale.x = scale
        self.icon_sprite.scale.y = scale

camera = CameraNode()

game_infos = []

def check_if_game_folder_and_track(path, sub_paths):
    # For a game to be runnable we need at most:
    #   * game_name         (displays somewhere around the launcher tile)
    #   * game_main_path    (path the the main file to run to start the game)
    #   * game_icon_path    (path to the .bmp to display the icon for the game)
    # but games can show up with their folder name and no icon if the folder
    # just contains a main.py (makes it easy to quickly prototype games)

    # The minimum a game folder needs to be a game is
    # either a main.py or a .ini file indicating
    # what files are what
    contains_ini = False
    contains_main_py = False
    for sub_path in sub_paths:
        if ".ini" in sub_path:
            contains_ini = True
            break
    
    if "main.py" in sub_paths:
        contains_main_py = True

    # Now that either of the above is true, we have the
    # beginning of a minimal game, fill the rest out
    if contains_main_py or contains_ini:
        # Now that we know it at least contains one of these,
        # create a info object that will need to be filled
        # out as best as possible
        info = GameInfo()

        # if a .ini file without a name field is
        # found, the folder name containing the file
        # will be used (do this by default every time)
        info.game_name = path[path.rfind("/")+1:]


        if contains_main_py:
            info.game_main_path = path + "/main.py"

        if "icon.bmp" in sub_paths:
            info.game_icon_path = path + "/icon.bmp"

        if contains_ini:
            ini_file_path = path + "/" + sub_path
            ini_file = open(ini_file_path, 'r')

            line_index = 0
            for line in ini_file:
                line = line.split("=")

                if len(line) == 0:
                    print("ERROR: .ini line does contain an equals sign", ini_file_path, "line: " + str(line_index))
                else:
                    # Remove any spaces on each side of the equals sign
                    key = line[0].replace(" ", "")
                    value = line[1].replace(" ", "")

                    # Remove any sort of new lines
                    key = key.replace("\r\n", "")
                    key = key.replace("\n", "")
                    value = value.replace("\r\n", "")
                    value = value.replace("\n", "")
                    
                    # Fill out the rest of the info object as well as possible
                    if key == "main":
                        info.game_main_path = path + "/" + value
                    elif key == "name":
                        info.game_name = value
                    elif key == "icon":
                        info.game_icon_path = path + "/" + value
                
                line_index += 1
            
            ini_file.close()

        # Track the game
        game_infos.append(info)


        



def search_paths_for_games(root, paths):
    # Loop through all paths and apply logic
    # to determine if the folder is a game
    # folder. Also check if sub-folders are game
    # folders
    for path in paths:
        # Need to determine these
        full_path = ""
        full_path_is_folder = False
        sub_paths = []

        # If the root folder is empty,
        # do not prepend a root slash
        if root == "":
            full_path = path
        else:
            full_path = root + "/" + path

        # Try to list all the files at the current path,
        # if it works, then the path is a folder and can
        # be checked to be containing more folders consisting
        # of games. Also check if the folder itself is a game
        # folder too, later
        try:
            sub_paths = os.listdir(full_path)
            search_paths_for_games(full_path, sub_paths)
            full_path_is_folder = True
        except Exception as e:
            pass
        
        # Now that we know that running listdir on a full_path
        # like "Games/MyGame" results in a list and not an error,
        # this path could give files that make it a game, check that
        # and track if this is a game folder
        if full_path_is_folder:
            check_if_game_folder_and_track(full_path, sub_paths)

search_paths_for_games("", os.listdir(""))

# Create the tiles in a list
index = 0
for game_info in game_infos:
    tile = LauncherTile()
    tile.position.x = (index * 45)
    tile.set_scale(0.3)
    tile.title_text_node.text = game_info.game_name
    tile.game_info = game_info

    if(game_info.game_icon_path != None):
        tile.icon_sprite.texture = TextureResource(game_info.game_icon_path)

    index += 1


# Toggle the gui elements and do not let the user
# toggle back out of it
engine_input.toggle_gui_focus()
engine_input.gui_toggle_button = None


engine.start()

execfile(game_path_to_execute)