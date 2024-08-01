from engine_nodes import Text2DNode

class LauncherSettingsScreen():
    def __init__(self, font):
        self.credit = Text2DNode(font=font, text="Nothing here\nyet...", letter_spacing=1, line_spacing=1)
        self.credit.position.x = 128