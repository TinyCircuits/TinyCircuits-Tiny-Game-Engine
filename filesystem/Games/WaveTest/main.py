import engine_main

import engine
import engine_audio
from engine_resources import WaveSoundResource
from engine_nodes import Circle2DNode, CameraNode

cir = Circle2DNode()
cam = CameraNode()

wave = WaveSoundResource("random.wav")

engine_audio.play(wave, 1, True)

engine.start()