import engine_main
import engine

from engine_resources import WaveSoundResource
import engine_audio
import engine_io
from engine_nodes import Circle2DNode, CameraNode

circle = Circle2DNode()
cam = CameraNode()

import os
os.chdir("Games/JingleTest")

wave = WaveSoundResource("ThumbyColorJingle1.wav")

while True:
    if engine.tick():
        if engine_io.A.is_just_pressed:
            engine_audio.play(wave, 0, False)