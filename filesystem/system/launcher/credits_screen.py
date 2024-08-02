from engine_nodes import Text2DNode

class CreditsScreen():
    def __init__(self, font):
        self.credit = Text2DNode(font=font, text="Ken Burns\n  - Money Man", letter_spacing=1, line_spacing=1)
        self.credit.position.x = 128*2