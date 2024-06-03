import engine_main

import engine
import engine_audio
from engine_resources import RTTTLSoundResource
from engine_nodes import Circle2DNode, CameraNode

cir = Circle2DNode(radius=25)
cam = CameraNode()

wave = RTTTLSoundResource("test.rtttl")

engine_audio.play(wave, 1, True)

engine.start()