import engine_main

import engine
import engine_draw
import engine_io

from engine_resources import TextureResource, FontResource
from engine_nodes import Sprite2DNode, CameraNode, GUIButton2DNode, Text2DNode
from engine_math import Vector2
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT

from system.launcher_state import get_launcher_state

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
# engine.freq(250 * 1000 * 1000)
# engine.fps_limit(60)

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

# Create instance of custom camera and add static
# elements as children so they don't move
camera = CustomCamera()
camera.add_child(screen_icon)
camera.add_child(header)
camera.add_child(dynamic_background)
camera.add_child(battery)

# Create instances of the custom launcher screens
games_screen = GamesScreen(font)
credits_screen = CreditsScreen(font)
settings_screen = SettingsScreen(font)

# Focus GUI layer by default, do not let user change
# focus manually, no navigation wrapping, and pass
# inputs through GUI layer (normally consumes everything)
engine_io.gui_focused(True)
engine_io.gui_wrapping(False)
engine_io.gui_passing(True)

# Get launcher state (TODO: do something with it)
launcher_state = get_launcher_state()

# Launcher page index
page = 0

# Main launcher loop
while True:
    if engine.tick():
        page_switched = False
        new_page = 0

        # Increment page in a direction
        if engine_io.RB.is_pressed_autorepeat:
            new_page = page + 1
            page_switched = True
        elif engine_io.LB.is_pressed_autorepeat:
            new_page = page - 1
            page_switched = True
        
        # Loop page of at either end
        if new_page >= 3:
            new_page = 0
        elif new_page < 0:
            new_page = 2
        
        # If the page was switched, and all animations
        # are complete, store the page index
        if page_switched == True and screen_icon.tween.finished and header.tween_0.finished and header.tween_1.finished and camera.tween.finished:
            screen_icon.to_page(new_page)
            header.to_page(new_page)
            camera.to_page(new_page)

            games_screen.tell_page(new_page)

            page = new_page