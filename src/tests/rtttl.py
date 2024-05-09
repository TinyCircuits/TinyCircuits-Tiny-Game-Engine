import engine_main

import engine
import engine_audio
from engine_nodes import Rectangle2DNode, CameraNode

from engine_resources import RTTTLSoundResource

rtttl = RTTTLSoundResource("castle.txt")

class Rect(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.width = 60
        self.height = 70

    def tick(self, dt):
        self.rotation += 0.01
r = Rect()
c = CameraNode()

engine_audio.play(rtttl, 0, True)

engine.start()