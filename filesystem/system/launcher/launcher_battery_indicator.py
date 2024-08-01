from engine_nodes import Rectangle2DNode, EmptyNode, Sprite2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2
import engine_draw
import engine_io
import math

rect_color = Color(0.631, 0.604, 0.796)
battery_tex = TextureResource("system/launcher/assets/battery.bmp")

class LauncherBatteryIndicator(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = battery_tex
        self.transparent_color=engine_draw.white
        self.frame_count_x=7
        self.playing=False
        self.layer=2

        self.position.x = -59
        self.position.y = 60
        self.update_indicator()
    
    def update_indicator(self):
        # Reset time tracker
        self.time = 0

        if engine_io.is_charging():
            self.frame_current_x = 6
        else:
            # Get level, adjust width and position of bar
            level = engine_io.battery_level()
            self.frame_current_x = math.floor((level * 6) + 0.5)


    def tick(self, dt):
        self.time += dt
        # Update the bar every second.
        if self.time >= 1:
            self.update_indicator()
            