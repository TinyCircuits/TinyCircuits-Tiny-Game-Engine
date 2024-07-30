from engine_draw import Color
from engine_nodes import Line2DNode, EmptyNode
from engine_math import Vector2

line_color = Color(0.631, 0.604, 0.796)

class LauncherHeader(EmptyNode):
    def __init__(self):
        super().__init__(self)

        self.left_horizontal = Line2DNode(color=line_color, start=Vector2(-70, -5), end=Vector2(-50, -5))
        self.mid_horizontal = Line2DNode(color=line_color, start=Vector2(-40, 0), end=Vector2(40, 0))
        self.right_horizontal = Line2DNode(color=line_color, start=Vector2(70, -5), end=Vector2(50, -5))

        self.left_to_mid = Line2DNode(color=line_color, start=self.left_horizontal.end, end=self.mid_horizontal.start)
        self.right_to_mid = Line2DNode(color=line_color, start=self.right_horizontal.end, end=self.mid_horizontal.end)

        self.add_child(self.left_horizontal)
        self.add_child(self.mid_horizontal)
        self.add_child(self.right_horizontal)

        self.add_child(self.left_to_mid)
        self.add_child(self.right_to_mid)

        self.position.y = -50
