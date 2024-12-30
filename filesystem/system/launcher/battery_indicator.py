from engine_nodes import Rectangle2DNode, EmptyNode, Sprite2DNode
from engine_draw import Color
from engine_resources import TextureResource
from engine_math import Vector2
import engine_draw
import engine_io
import math

battery_tex = TextureResource("system/launcher/assets/battery.bmp")

class BatteryIndicator(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = battery_tex
        self.transparent_color=engine_draw.white
        self.frame_count_x=7
        self.playing=False
        self.layer=2

        self.position.x = 39
        self.position.y = -45
        self.level_readings = [0, 0, 0, 0, 0]
        self.was_charging = True
        self.time = 0


    def take_reading(self):
        # Get newest reading
        new_level = engine_io.battery_level()

        # Remove oldest reading and add new one to end
        self.level_readings.pop(0)
        self.level_readings.append(new_level)


    def update_indicator(self):
        # Init avg level for calculation
        avg_level = 0

        # Sum and calculate the average (reduces flicker)
        for level in self.level_readings:
            avg_level += level

        avg_level = avg_level / len(self.level_readings)

        # Adjust sprite frame
        self.frame_current_x = math.ceil(avg_level * 5)


    def tick(self, dt):
        # If charging, nothing to do except make sure
        # to display the charging symbol
        if engine_io.is_charging():
            self.was_charging = True
            self.frame_current_x = 6
            return

        # Is not charging now but was charging before,
        # refill the readings right now and update the
        # indicator (fixes showing different level when
        # unplugging or launching the launcher)
        if engine_io.is_charging() is False and self.was_charging:
            self.was_charging = False

            for i in range(len(self.level_readings)):
                self.take_reading()

            # Update the indicator based on avg. readings
            self.update_indicator()

        # Update the readings every half second when not charging
        self.time += dt

        if self.time >= 0.5:
            # Reset time tracker
            self.time = 0

            # Take a new reading
            self.take_reading()

            # Update the indicator based on avg. readings
            self.update_indicator()
            