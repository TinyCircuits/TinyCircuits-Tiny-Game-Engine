from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, EmptyNode
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT
from engine_math import Vector2
import engine_draw

import math

thumby_color_icon_tex = TextureResource("system/launcher/assets/launcher-screen-thumby-color-icon.bmp")
gear_icon_tex = TextureResource("system/launcher/assets/launcher-screen-gear-icon.bmp")
credits_icon_tex = TextureResource("system/launcher/assets/launcher-screen-credits-icon.bmp")

class ScreenIcon(EmptyNode):
    def __init__(self):
        super().__init__(self)

        element_count = 3
        self.angle_inc = math.radians(360/3)

        self.thumby_color_spr = Sprite2DNode(texture=thumby_color_icon_tex, opacity=0.65, scale=Vector2(0.75, 0.75), rotation=-math.pi/4)
        self.gear_spr         = Sprite2DNode(texture=gear_icon_tex, opacity=0.65, scale=Vector2(0.75, 0.75))

        self.credits_heart_group = EmptyNode()
        self.credits_heart_spr_0 = Sprite2DNode(texture=credits_icon_tex, opacity=0.65, scale=Vector2(0.375, 0.375), rotation=self.angle_inc+math.pi/4, transparent_color=engine_draw.white)
        self.credits_heart_spr_1 = Sprite2DNode(texture=credits_icon_tex, opacity=0.65, scale=Vector2(0.325, 0.325), rotation=self.angle_inc+math.pi/6, transparent_color=engine_draw.white)
        self.credits_heart_spr_2 = Sprite2DNode(texture=credits_icon_tex, opacity=0.65, scale=Vector2(0.275, 0.275), rotation=self.angle_inc+math.pi/3.5, transparent_color=engine_draw.white)
        self.credits_heart_group.add_child(self.credits_heart_spr_0)
        self.credits_heart_group.add_child(self.credits_heart_spr_1)
        self.credits_heart_group.add_child(self.credits_heart_spr_2)

        self.thumby_color_spr.position.x = 64 * math.cos(self.angle_inc*2)
        self.thumby_color_spr.position.y = 64 * math.sin(self.angle_inc*2)

        self.gear_spr.position.x = 64 * math.cos(0)
        self.gear_spr.position.y = 64 * math.sin(0)

        self.credits_heart_group.position.x = 64 * math.cos(self.angle_inc)
        self.credits_heart_group.position.y = 64 * math.sin(self.angle_inc)
        self.credits_heart_spr_0.position.x = -4
        self.credits_heart_spr_0.position.y = 22

        self.credits_heart_spr_1.position.x = -25
        self.credits_heart_spr_1.position.y = 34

        self.credits_heart_spr_2.position.x = 8
        self.credits_heart_spr_2.position.y = 43
        
        self.add_child(self.thumby_color_spr)
        self.add_child(self.gear_spr)
        self.add_child(self.credits_heart_group)
        
        self.position.x = 102
        self.position.y = 123

        self.tween = Tween()
    
    def tick(self, dt):
        speed = 0.25            # radians/s
        increment = speed * dt  # radians
        self.gear_spr.rotation += increment
    
    def to_page(self, page_index):
        if self.tween.finished == False:
            return False
        
        self.tween.start(self.rotation, "z", self.rotation.z, page_index*self.angle_inc, 250, 1.0, ONE_SHOT, EASE_SINE_OUT)
        return True