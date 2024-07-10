import engine_draw
import engine_io
import time
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Text2DNode
from engine_resources import TextureResource
from tilemapper import TTileType, TileRule, TileRules, TilingRenderer

tile_rules = TileRules()

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    [1, 1, 1]
], (1, 1))) 

tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    [1, 1, 1],
    [1, 1, 1]
], (1, 0)))

tile_rules.add_rule(TileRule([
    ['*', 1, 1],
    ['*', 1, 1],
    ['*', 1, 1]
], (0, 1)))

tile_rules.add_rule(TileRule([
    [1, 1, '*'],
    [1, 1, '*'],
    [1, 1, '*']
], (2, 1)))

tile_rules.add_rule(TileRule([
    [1, 1, 1],
    [1, 1, 1],
    ['*', '*', '*']
], (1, 2)))

tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    [1, 1, '*'],
    [1, 1, '*']
], (2, 0)))

tile_rules.add_rule(TileRule([
    [1, 1, '*'],
    [1, 1, '*'],
    ['*', '*', '*']
], (2, 2)))

tile_rules.add_rule(TileRule([
    ['*', 1, 1],
    ['*', 1, 1],
    ['*', '*', '*']
], (0, 2)))

tile_rules.add_rule(TileRule([
    ['*', '*', '*'],
    ['*', 1, 1],
    ['*', 1, 1]
], (0, 0)))

class SimpleTextMenu(Rectangle2DNode):
    def __init__(self, title, options, callback, font, disp_width, disp_height, text_color, selected_color):
        super().__init__(self)
        self.width = disp_width
        self.height = disp_height
        self.position = Vector2(0, 0)
        self.color = engine_draw.black
        self.text_nodes = []
        self.title = title
        self.options = options
        self.selected_option = 0
        self.callback = callback
        self.text_color = text_color
        self.selected_color = selected_color
        self.font = font
        #bordertexture = TextureResource("panel9slice.bmp")
        #bordertile = TTileType('grass', bordertexture, frame_count_x=3, frame_count_y=3)
        #width_div_5 = disp_width // 5
        #height_div_5 = disp_height // 5
        #array_of_ones = [[1 for _ in range(width_div_5)] for _ in range(height_div_5)]
        #self.backgroundborder = TilingRenderer(array_of_ones, bordertile, tile_rules)
        #self.backgroundborder.layer = 1
        #self.add_child(self.backgroundborder)
        self.create_menu()

    def create_menu(self):
        title_text = Text2DNode(position=Vector2(self.width / 2, self.height / 4),
                                font=self.font,
                                text=self.title,
                                rotation=0,
                                scale=Vector2(1, 1),
                                opacity=1.0,
                                letter_spacing=1,
                                line_spacing=1)
        title_text.color = self.text_color
        title_text.layer = 2
        self.add_child(title_text)

        for i, option in enumerate(self.options):
            y_position = self.height / 2 + i * 10
            text_node = Text2DNode(position=Vector2(self.width / 2, y_position),
                                   font=self.font,
                                   text=option,
                                   rotation=0,
                                   scale=Vector2(1, 1),
                                   opacity=1.0,
                                   letter_spacing=1,
                                   line_spacing=1)
            text_node.color = self.text_color if i != self.selected_option else self.selected_color
            text_node.layer = 2
            self.add_child(text_node)
            self.text_nodes.append(text_node)

    def update_selection(self):
        for i, text_node in enumerate(self.text_nodes):
            text_node.color = self.text_color if i != self.selected_option else self.selected_color

    def tick(self, dt):
        if engine_io.UP.is_just_pressed:
            self.selected_option = max(0, self.selected_option - 1)
            self.update_selection()
        elif engine_io.DOWN.is_just_pressed:
            self.selected_option = min(len(self.options) - 1, self.selected_option + 1)
            self.update_selection()
        elif engine_io.A.is_just_pressed:
            time.sleep_ms(1000)
            self.callback(self.selected_option)
            self.position.x = self.position.x -200
            #self.destroy()