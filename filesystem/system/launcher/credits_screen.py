from engine_nodes import Text2DNode
from engine_math import Vector2

files = ["system/credits_1_testers.csv", "system/credits_2_collectors.csv", "system/credits_3_special.csv"]

class Name(Text2DNode):
    def __init__(self, font_texture_resource, y_inc):
        super().__init__(self)
        self.position.x = 128*2
        self.position.y = 64+(font_texture_resource.height/2)+y_inc*i

class CreditsScreen():
    def __init__(self, font_texture_resource):
        self.scrolling_nodes = []

        # row_count = int(128.0 / (font_texture_resource.height + 5))

        # y_inc = 128.0/row_count

        # for i in range(row_count):
        #     self.scrolling_nodes.append(Text2DNode(font=font_texture_resource, text="Test", letter_spacing=1, position=Vector2(128*2, 64+(font_texture_resource.height/2)+y_inc*i)))