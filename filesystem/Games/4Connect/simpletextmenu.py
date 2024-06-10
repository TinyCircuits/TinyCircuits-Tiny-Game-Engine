import engine_draw
import engine_io
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Text2DNode

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
        title_text.set_layer(1)
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
            text_node.set_layer(1)
            self.add_child(text_node)
            self.text_nodes.append(text_node)

    def update_selection(self):
        for i, text_node in enumerate(self.text_nodes):
            text_node.color = self.text_color if i != self.selected_option else self.selected_color

    def tick(self, dt):
        if engine_io.check_just_pressed(engine_io.DPAD_UP):
            self.selected_option = max(0, self.selected_option - 1)
            self.update_selection()
        elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
            self.selected_option = min(len(self.options) - 1, self.selected_option + 1)
            self.update_selection()
        elif engine_io.check_just_pressed(engine_io.A):
            self.callback(self.selected_option)
            self.destroy()