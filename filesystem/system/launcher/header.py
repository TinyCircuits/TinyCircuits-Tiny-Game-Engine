import engine_draw
from engine_draw import Color
from engine_nodes import Sprite2DNode, EmptyNode, Rectangle2DNode, Text2DNode
from engine_math import Vector2
from engine_resources import TextureResource, FontResource
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT
import engine_io
from system.launcher.direction_icon import DirectionIcon

rect_color = Color(0.631, 0.604, 0.796)

header_island_tex = TextureResource("system/launcher/assets/launcher-header-island.bmp")
games_header_tex = TextureResource("system/launcher/assets/launcher-games-header.bmp")
settings_header_tex = TextureResource("system/launcher/assets/launcher-settings-header.bmp")
credits_header_tex = TextureResource("system/launcher/assets/launcher-credits-header.bmp")

class Header(EmptyNode):
    def __init__(self, font):
        super().__init__(self)

        # Create sprites and position the,
        self.header_island_spr = Sprite2DNode(texture=header_island_tex, transparent_color=engine_draw.white, opacity=0.65, layer=3)

        self.LB = DirectionIcon(Text2DNode(text="LB", font=font, layer=3), engine_io.LB, Vector2(-54, 1))
        self.RB = DirectionIcon(Text2DNode(text="RB", font=font, layer=3), engine_io.RB, Vector2(54, 1))

        self.games_header_spr = Sprite2DNode(texture=games_header_tex, transparent_color=engine_draw.white, layer=3)
        self.settings_header_spr = Sprite2DNode(texture=settings_header_tex, transparent_color=engine_draw.white, position=Vector2(0, -36), layer=3)
        self.credits_header_spr = Sprite2DNode(texture=credits_header_tex, transparent_color=engine_draw.white, position=Vector2(0, -36), layer=3)

        self.position.y = -57

        # Add sprites as children of empty node parent
        self.add_child(self.header_island_spr)
        self.add_child(self.LB)
        self.add_child(self.RB)
        self.add_child(self.games_header_spr)
        self.add_child(self.settings_header_spr)
        self.add_child(self.credits_header_spr)

        # Add sprites to list
        self.headers = []
        self.headers.append(self.games_header_spr)
        self.headers.append(self.settings_header_spr)
        self.headers.append(self.credits_header_spr)

        # Create an index into the headers list and create tween objects
        self.last_page_index = 0
        self.tween_0 = Tween()
        self.tween_1 = Tween()

        # Create little page index indicators
        self.page_indicators = []
        self.page_indicators.append(Rectangle2DNode(position=Vector2(-5-4, 12), width=7, height=3, opacity=0.5, layer=2))
        self.page_indicators.append(Rectangle2DNode(position=Vector2(0,    12), width=7, height=3, opacity=0.15, layer=2))
        self.page_indicators.append(Rectangle2DNode(position=Vector2(5+4,  12), width=7, height=3, opacity=0.15, layer=2))
        self.add_child(self.page_indicators[0])
        self.add_child(self.page_indicators[1])
        self.add_child(self.page_indicators[2])
    
    def to_page(self, page_index):
        if self.tween_0.finished == False or self.tween_1.finished == False:
            return False

        self.tween_0.start(self.headers[self.last_page_index].position, "y", 0.0, -36.0, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)
        self.page_indicators[self.last_page_index].opacity = 0.15

        self.tween_1.start(self.headers[page_index].position, "y", -36.0, 0.0, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)
        self.page_indicators[page_index].opacity = 0.5

        self.last_page_index = page_index

        return True


