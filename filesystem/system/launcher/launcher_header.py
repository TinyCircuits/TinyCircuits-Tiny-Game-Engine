import engine_draw
from engine_nodes import Line2DNode, Sprite2DNode, EmptyNode
from engine_math import Vector2
from engine_resources import TextureResource
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT

header_island_tex = TextureResource("system/launcher/assets/launcher-header-island.bmp")
games_header_tex = TextureResource("system/launcher/assets/launcher-games-header.bmp")
settings_header_tex = TextureResource("system/launcher/assets/launcher-settings-header.bmp")
credits_header_tex = TextureResource("system/launcher/assets/launcher-credits-header.bmp")


class LauncherHeader(EmptyNode):
    def __init__(self):
        super().__init__(self)

        # Create sprites and position the,
        self.header_island_spr = Sprite2DNode(texture=header_island_tex, transparent_color=engine_draw.white)

        self.games_header_spr = Sprite2DNode(texture=games_header_tex, transparent_color=engine_draw.white)
        self.games_header_spr.position.y = -6

        self.settings_header_spr = Sprite2DNode(texture=settings_header_tex, transparent_color=engine_draw.white)
        self.settings_header_spr.position.y = -32

        self.credits_header_spr = Sprite2DNode(texture=credits_header_tex, transparent_color=engine_draw.white)
        self.credits_header_spr.position.y = -32

        self.position.y = -51

        # Add sprites as children of empty node parent
        self.add_child(self.header_island_spr)
        self.add_child(self.games_header_spr)
        self.add_child(self.settings_header_spr)
        self.add_child(self.credits_header_spr)

        # Add sprites to list
        self.headers = []
        self.headers.append(self.games_header_spr)
        self.headers.append(self.settings_header_spr)
        self.headers.append(self.credits_header_spr)

        # Create an index into the headers list and create tween objects
        self.index = 0
        self.tween_0 = Tween()
        self.tween_1 = Tween()
    
    def inc(self):
        if self.tween_0.finished == False or self.tween_1.finished == False:
            return

        self.tween_0.start(self.headers[self.index].position, "y", -6.0, -36.0, 350, 1.0, ONE_SHOT, EASE_SINE_OUT)

        self.index += 1

        if(self.index >= 2):
            self.index = 0

        self.tween_1.start(self.headers[self.index].position, "y", -36.0, -6.0, 350, 1.0, ONE_SHOT, EASE_SINE_OUT)



