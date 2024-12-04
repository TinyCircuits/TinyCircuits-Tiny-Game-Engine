import engine_io
from engine_nodes import Text2DNode
from engine_nodes import EmptyNode, Rectangle2DNode, Text2DNode, GUIBitmapButton2DNode, Sprite2DNode
from engine_math import Vector2
from engine_resources import TextureResource
import engine_draw
import engine
from engine_draw import Color

setting_background_color = Color(0.157, 0.137, 0.263)                          # Background color for category rows
bar_color = Color(0.757, 0.737, 0.978)
volume_texture = TextureResource("system/launcher/assets/volume.bmp")
sun_texture = TextureResource("system/launcher/assets/sun.bmp")

page = 0


class SettingIcon(GUIBitmapButton2DNode):
    def __init__(self, font, bitmap, on_focus, on_unfocus):
        super().__init__(self)

        self.inherit_scale = False
        self.inherit_opacity = False
        self.font = font
        self.text = " "
        self.bitmap = bitmap
        self.transparent_color = engine_draw.white

        self.on_focus = on_focus
        self.on_unfocus = on_unfocus
    
    def on_before_focused(self):
        if page == 1:
            return True
        else:
            return False
    
    def on_just_focused(self):
        self.on_focus()
    
    def on_just_unfocused(self):
        self.on_unfocus()



class SettingsSlider(Rectangle2DNode):
    def __init__(self, font, bitmap, min, max, setter_getter):
        super().__init__(self)

        self.min = min
        self.max = max

        self.setter_getter = setter_getter
        self.percentage = self.setter_getter()
        
        self.scale_factor = 0.65

        self.width = 120
        self.height = (bitmap.height*self.scale_factor)+2
        self.layer = 0
        self.inherit_scale=False
        self.color=setting_background_color
        self.opacity=0.35
        self.position.x = 128

        self.icon = SettingIcon(font, bitmap, self.on_focus, self.on_unfocus)
        self.icon.scale.x = self.scale_factor
        self.icon.scale.y = self.scale_factor
        self.icon.position.x = (-self.width/2) + (bitmap.width*self.scale_factor/2) + 1

        self.bar_max_width = self.width - (bitmap.width*self.scale_factor) - 6
        self.bar = Rectangle2DNode(width=self.bar_max_width*self.percentage, height=self.height-14, inherit_scale=False, inherit_opacity=False, color=bar_color)
        self.position_bar()

        self.add_child(self.icon)
        self.add_child(self.bar)
    
    def position_bar(self):
        self.bar.position.x =  -(self.width/2) + (self.icon.bitmap.width*self.scale_factor) + (self.bar.width/2) + 4

    def focus(self):
        self.icon.focused = True
    
    def unfocus(self):
        self.icon.focused = False

    def on_focus(self):
        self.color = engine_draw.black
    
    def on_unfocus(self):
        self.color = setting_background_color

    def update(self):
        if self.percentage > self.max:
            self.percentage = self.max
        elif self.percentage < self.min:
            self.percentage = self.min

        self.bar.width = self.bar_max_width * self.percentage
        self.position_bar()

        self.setter_getter(self.percentage, False)  # Do not save setting, just apply

    def tick(self, dt):
        if self.icon.focused is not True:
            return
        
        if engine_io.LEFT.is_pressed:
            self.percentage -= 0.025
            self.update()
        elif engine_io.RIGHT.is_pressed:
            self.percentage += 0.025
            self.update()
        
        # Only save after one or the other button is done being held
        # (don't want to constantly write the file)
        if engine_io.LEFT.is_just_released or engine_io.RIGHT.is_just_released:
            self.setter_getter(self.percentage, True)   # Actually save the setting
        

class SettingsScreen():
    def __init__(self, font):
        self.volume_slider = SettingsSlider(font, volume_texture, 0.0, 1.0, engine.setting_volume)
        self.volume_slider.position.y = -28

        self.brightness_slider = SettingsSlider(font, sun_texture, 0.05, 1.0, engine.setting_brightness)
        self.brightness_slider.position.y = -2
    
    def tell_page(self, new_page):
        global page
        page = new_page

        if page == 1:
            self.volume_slider.focus()
        else:
            self.volume_slider.unfocus()
            self.brightness_slider.unfocus()