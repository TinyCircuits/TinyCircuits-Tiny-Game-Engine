from engine_draw import Color
from engine_nodes import Rectangle2DNode, EmptyNode
from engine_math import Vector2

import math

rect_color = Color(0.631, 0.604, 0.796)

class FadeRectangle(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.width = 21
        self.height = 6
        self.color = rect_color
        self.opacity = 0.5
        self.time = 0

    def tick(self, dt):
        opacity = math.sin(self.time + self.position.x/5) # -1.0 ~ 1.0
        opacity = 0.5 + 0.5*opacity                       #  0.0 ~ 1.0
        opacity = opacity*0.15                            #  0.0 ~ 0.15
        opacity += 0.1                                    #  0.1 ~ 0.25

        self.opacity = opacity
        self.time += dt


class DynamicBackground(EmptyNode):
    def __init__(self):
        super().__init__(self)

        self.rectangles = []

        start_x = -30*2

        for i in range(5):
            rect = FadeRectangle()
            rect.position.x = start_x+30*i
            self.rectangles.append(rect)
            self.add_child(rect)
        
        start_x = -45

        for i in range(4):
            rect = FadeRectangle()
            rect.position.x = start_x+30*i
            rect.position.y = 13
            self.rectangles.append(rect)
            self.add_child(rect)
        
        start_x = -30

        for i in range(3):
            rect = FadeRectangle()
            rect.position.x = start_x+30*i
            rect.position.y = 26
            self.rectangles.append(rect)
            self.add_child(rect)
        
        self.rotation.z = math.pi/4

        self.position.x = 8
        self.position.y = 8