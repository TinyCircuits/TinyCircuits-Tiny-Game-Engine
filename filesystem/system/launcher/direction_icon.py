import engine_io
from engine_nodes import EmptyNode

class DirectionIcon(EmptyNode):
    def __init__(self, node, button, position_2d):
        super().__init__(self)
        self.position.x = position_2d.x
        self.position.y = position_2d.y
        self.node = node
        self.button = button
        self.add_child(self.node)

        self.disabled = False
    
    def tick(self, dt):
        if not self.disabled:
            if self.button.is_pressed:
                self.node.opacity = 0.65
            else:
                self.node.opacity = 1.0
        else:
            self.node.opacity = 0.0