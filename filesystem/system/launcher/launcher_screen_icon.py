from engine_resources import TextureResource
from engine_nodes import Sprite2DNode, EmptyNode
from engine_animation import Tween, ONE_SHOT, EASE_BACK_OUT, EASE_SINE_OUT

import math

thumby_color_icon_tex = TextureResource("system/launcher/assets/launcher-screen-thumby-color-icon.bmp")
gear_icon_tex = TextureResource("system/launcher/assets/launcher-screen-gear-icon.bmp")
credits_icon_tex = TextureResource("system/launcher/assets/launcher-screen-credits-icon.bmp")

class LauncherScreenIcon(EmptyNode):
    def __init__(self):
        super().__init__(self)

        self.thumby_color_spr = Sprite2DNode(texture=thumby_color_icon_tex, opacity=0.65)
        self.gear_spr = Sprite2DNode(texture=gear_icon_tex, opacity=0.65)
        self.credits_spr = Sprite2DNode(texture=credits_icon_tex, opacity=0.65)
        

    #     self.gear_spr.scale.x = 0.75
    #     self.gear_spr.scale.y = 0.75
    #     self.gear_spr.position.x = 45

    #     self.thumby_color_spr.scale.x = 0.75
    #     self.thumby_color_spr.scale.y = 0.75
    #     self.thumby_color_spr.position.x = -45

    #     self.add_child(self.gear_spr)
    #     self.add_child(self.thumby_color_spr)

    #     self.rotation.z = -math.pi/4
    #     self.position.x = 100
    #     self.position.y = 100

    #     self.tween = Tween()
    #     self.time = 0
    
    # def tick(self, dt):
    #     speed = 0.25            # radians/s
    #     increment = speed * dt  # radians
    #     self.gear_spr.rotation += increment
    
    # def inc(self):
    #     if self.tween.finished == True:
    #         self.tween.start(self.rotation, "z", self.rotation.z, self.rotation.z+math.pi, 350, 1.0, ONE_SHOT, EASE_SINE_OUT)