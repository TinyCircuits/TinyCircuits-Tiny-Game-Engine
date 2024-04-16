import engine
import engine_audio
import engine_input
from engine_nodes import CameraNode

from engine_resources import WaveSoundResource


wave0 = WaveSoundResource("e0.wav")
wave1 = WaveSoundResource("e1.wav")
wave2 = WaveSoundResource("e2.wav")
wave3 = WaveSoundResource("e3.wav")


camera = CameraNode()


while True:
    
    if engine_input.check_just_pressed(engine_input.BUMPER_RIGHT):
        c = engine_audio.play(wave0, 0, False)
        c.gain = 1
        
    if engine_input.check_just_pressed(engine_input.BUMPER_LEFT):
        c = engine_audio.play(wave1, 1, False)
        c.gain = 1
    
    if engine_input.check_just_pressed(engine_input.A):
        c = engine_audio.play(wave2, 2, False)
        c.gain = 1
        
    if engine_input.check_just_pressed(engine_input.B):
        c = engine_audio.play(wave3, 3, False)
        c.gain = 1
    
    engine.tick()
