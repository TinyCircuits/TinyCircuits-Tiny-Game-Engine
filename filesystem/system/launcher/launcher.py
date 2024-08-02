import engine_main

import engine
import engine_draw
import engine_io

from engine_resources import TextureResource, FontResource
from engine_nodes import Sprite2DNode, CameraNode, GUIButton2DNode, Text2DNode
from engine_math import Vector2
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT

from system.launcher.screen_icon import ScreenIcon
from system.launcher.header import Header
from system.launcher.dynamic_background import DynamicBackground
from system.launcher.battery_indicator import BatteryIndicator
from system.launcher.games_screen import GamesScreen
from system.launcher.credits_screen import CreditsScreen
from system.launcher.settings_screen import SettingsScreen
from system.launcher.custom_camera import CustomCamera

# Speed up the processor to draw the launcher at a 
# higher speed and set an FPS limit
engine.freq(225 * 1000 * 1000)
engine.fps_limit(60)

# Load some resources and set background 
font = FontResource("system/assets/outrunner_outline.bmp")
background_tex = TextureResource("system/launcher/assets/launcher-background.bmp")
engine_draw.set_background(background_tex)

# Create instances of some custom launcher UI and
# decorations
screen_icon = ScreenIcon()
header = Header(font)
dynamic_background = DynamicBackground()
battery = BatteryIndicator()

# Create instances of the custom launcher screens
games_screen = GamesScreen(font)
credits_screen = CreditsScreen(font)
settings_screen = SettingsScreen(font)

# Create instance of custom camera and add static
# elements as children so they don't move
camera = CustomCamera()
camera.add_child(screen_icon)
camera.add_child(header)
camera.add_child(dynamic_background)
camera.add_child(battery)

# Focus GUI layer by default, do not let user change
# focus manually, no navigation wrapping, and pass
# inputs through GUI layer (normally consumes everything)
engine_io.gui_focused(True)
engine_io.gui_toggle_button(None)
engine_io.gui_wrapping(False)
engine_io.gui_passing(True)

# Main launcher loop
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