import engine_main

import engine
import engine_draw
import engine_io

from engine_resources import TextureResource, FontResource
from engine_nodes import Sprite2DNode, CameraNode, GUIButton2DNode, Text2DNode
from engine_math import Vector2
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT

from system.launcher.launcher_screen_icon import LauncherScreenIcon
from system.launcher.launcher_header import LauncherHeader
from system.launcher.launcher_dynamic_background import LauncherDynamicBackground
from system.launcher.launcher_battery_indicator import LauncherBatteryIndicator
from system.launcher.launcher_games_screen import LauncherGamesScreen
from system.launcher.launcher_credits_screen import LauncherCreditsScreen
from system.launcher.launcher_settings_screen import LauncherSettingsScreen

engine.fps_limit(60)

font = FontResource("system/assets/outrunner_outline.bmp")

background_tex = TextureResource("system/launcher/assets/launcher-background.bmp")
engine_draw.set_background(background_tex)

screen_icon = LauncherScreenIcon()
header = LauncherHeader(font)
dynamic_background = LauncherDynamicBackground()
battery = LauncherBatteryIndicator()

class LauncherCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.add_child(screen_icon)
        self.add_child(header)
        self.add_child(dynamic_background)
        self.add_child(battery)
        self.camera_tween = Tween()
        self.index = 0
    
    def switch(self, dir):
        if self.camera_tween.finished == False:
            return

        self.index += dir

        if(self.index >= 3):
            self.index = 0
        elif(self.index < 0):
            self.index = 2

        self.camera_tween.start(self.position, "x", self.position.x, self.index*128.0, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)

    def goto_x(self, x):
        self.camera_tween.start(self.position, "x", self.position.x, x, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)
    
    def goto_y(self, y):
        self.camera_tween.start(self.position, "y", self.position.y, y, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)


camera = LauncherCamera()

games_screen = LauncherGamesScreen(font, camera)
credits_screen = LauncherCreditsScreen(font)
settings_screen = LauncherSettingsScreen(font)

# Toggle the gui elements and do not let the user toggle back out of it.
# engine_io.gui_focused(True)
# engine_io.gui_toggle_button(None)

while True:
    if engine.tick():
        if engine_io.RB.is_pressed_autorepeat:
            screen_icon.switch(1)
            header.switch(1)
            camera.switch(1)
        elif engine_io.LB.is_pressed_autorepeat:
            screen_icon.switch(-1)
            header.switch(-1)
            camera.switch(-1)



# engine.start()

# launcher_tile_texture = 
# launcher_tile_mark_texture = TextureResource("system/launcher/assets/launcher-tile-mark.bmp")


# import time
# import os
# from engine_math import Vector2, Vector3
# from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT
# from system.run_on_boot import set_run_on_boot
# from system.util import basename, thumby_reset, is_dir, is_file

# class GameInfo():
#     def __init__(self, dir):
#         self.dir = dir
#         self.name = basename(dir)
#         self.main_path = f"{dir}/main.py"
#         self.icon_path = None
#         self.mode = "thumby-color"

#     def __repr__(self):
#         return f"{self.dir} {{main={self.main_path}, icon={self.icon_path}}}"

# class GameDirListInfo():
#     def __init__(self, dir):
#         self.dir = dir
#         self.name = basename(dir)
#         self.games = []

#     def __repr__(self):
#         return f"{self.name or "(group)"}: {self.games}"

# def get_game_in_dir(dir):
#     """Returns the GameInfo representing the game in the given dir, if there is one. Does not check subdirs."""
#     entries = os.listdir(dir)
#     if "manifest.ini" in entries or "main.py" in entries:
#         gi = GameInfo(dir)
#         if "manifest.ini" in entries:
#             manifest_path = f"{dir}/manifest.ini"
#             with open(manifest_path) as f:
#                 for line in f:
#                     line = line.strip()
#                     if not line:
#                         continue
#                     eq_pos = line.find("=")
#                     if eq_pos < 0:
#                         print(f"ERROR: Bad line in manifest file {manifest_path}: {repr(line)}")
#                     key = line[:eq_pos].strip()
#                     value = line[eq_pos+1:].strip()
#                     if key == "name":
#                         gi.name = value
#                     elif key == "main":
#                         gi.main_path = f"{dir}/{value}"
#                     elif key == "icon":
#                         gi.icon_path = f"{dir}/{value}"
#                     elif key == "mode":
#                         gi.mode = value
#                     # Ignore unrecognized keys
#         if not gi.icon_path:
#             def_icon_path = f"{dir}/icon.bmp"
#             if is_file(def_icon_path):
#                 gi.icon_path = def_icon_path
#         return gi
#     name = basename(dir)
#     if f"{name}.py" in entries:
#         # A game in legacy mode.
#         gi = GameInfo(dir)
#         gi.main_path = f"{dir}/{name}.py"
#         gi.mode = "thumby-1"
#         return gi
#     return None

# def get_games_in_dir(dir):
#     """Returns the GameInfo or GameDirListInfo representing the game(s) in the given dir."""
#     game = get_game_in_dir(dir)
#     if game:
#         return game
#     gli = GameDirListInfo(dir)
#     for entry in os.listdir(dir or "."):
#         subdir = f"{dir}/{entry}"
#         if is_dir(subdir):
#             games_in_subdir = get_games_in_dir(subdir)
#             if games_in_subdir:
#                 gli.games.append(games_in_subdir)
#     gli.games.sort(key=lambda gi: gi.name.lower())
#     return gli if gli.games else None

# games = get_games_in_dir("Games")

# engine.fps_limit(60)

# font = FontResource("/system/assets/outrunner_outline.bmp")
# launcher_tile_texture = TextureResource("/system/launcher/assets/launcher-tile.bmp")
# launcher_tile_mark_texture = TextureResource("/system/launcher/assets/launcher-tile-mark.bmp")




# battery = BatteryIndicator()

# camera = CameraNode()
# camera_tween = Tween()
# camera.add_child(battery)

# tiles = []

# class LauncherTile(GUIBitmapButton2DNode):
#     def __init__(self, game_info, focused=False):
#         super().__init__(self)
#         tiles.append(self)

#         self.focused = focused
#         self.initial_focused = focused

#         self.game_info = game_info

#         self.font = font
#         self.text = ""
#         self.bitmap = launcher_tile_texture

#         self.transparent_color = engine_draw.white
#         self.scale.x = 0.35
#         self.scale.y = 0.35
#         self.layer = 2

#         self.title_text_node = Text2DNode(text=game_info.name, font=font, position=Vector2(0, -28), opacity=0.0, letter_spacing=1.0)

#         self.icon_sprite = Sprite2DNode(texture=launcher_tile_mark_texture)
#         self.icon_sprite.scale.x = 0.35
#         self.icon_sprite.scale.y = 0.35
#         self.icon_sprite.layer = 0
#         if game_info.icon_path:
#             texture = TextureResource(game_info.icon_path)
#             self.icon_sprite.texture = texture
#             self.icon_sprite.scale.x = (texture.width/self.bitmap.width) + 0.05
#             self.icon_sprite.scale.y = (texture.height/self.bitmap.height) + 0.05

#         self.add_child(self.icon_sprite)
#         self.add_child(self.title_text_node)

#         self.set_opacity(0.65)

#     def on_just_focused(self):
#         self.set_opacity(1.0)
#         self.set_scale(0.4)
#         self.title_text_node.opacity = 1.0
#         if self.initial_focused:
#             # If focused initially, don't animate the camera.
#             self.initial_focused = False
#             camera_tween.stop()
#             camera.position.x = self.position.x
#             camera.position.y = self.position.y
#         else:
#             camera_tween.start(camera, "position",
#                 None, Vector3(self.position.x, self.position.y, camera.position.z),
#                 250.0, 1.0, ONE_SHOT, EASE_BACK_OUT)

#     def on_just_unfocused(self):
#         self.set_opacity(0.65)
#         self.set_scale(0.3)
#         self.position.y = 0
#         self.title_text_node.opacity = 0.0

#     def on_just_pressed(self):
#         engine_io.rumble(0.2)
#         time.sleep(0.1)
#         engine_io.rumble(0.0)
#         set_run_on_boot(self.game_info.main_path)
#         thumby_reset(False)

#     def set_opacity(self, opacity):
#         self.opacity = opacity
#         # This shouldn't be necessary as it's a child of self, but it is for now.
#         self.icon_sprite.opacity = opacity

#     def set_scale(self, scale):
#         self.scale.x = scale
#         self.scale.y = scale
#         # This shouldn't be necessary as it's a child of self, but it is for now.
#         self.icon_sprite.scale.x = scale
#         self.icon_sprite.scale.y = scale


# index = 0

# def create_tile(game_info, focused=False):
#     global index
#     tile = LauncherTile(game_info, focused)
#     tile.position.x = (index * 42)
#     index += 1
#     tile.set_scale(0.3)

# # Traverse games and create a tile for each game, ignoring the game lists structure for now.

# def add_games(game_or_list):
#     if isinstance(game_or_list, GameInfo):
#         create_tile(game_or_list)
#     else:
#         for game in game_or_list.games:
#             add_games(game)

# add_games(games)

# # Toggle the gui elements and do not let the user toggle back out of it.
# engine_io.gui_focused(True)
# engine_io.gui_toggle_button(None)

# engine.start()
