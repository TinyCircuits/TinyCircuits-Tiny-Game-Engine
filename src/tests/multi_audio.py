import engine
import engine_audio
import engine_io
from engine_nodes import CameraNode

from engine_resources import WaveSoundResource, ToneSoundResource


wave0 = WaveSoundResource("e0.wav")
tone0 = ToneSoundResource()
wave2 = WaveSoundResource("8bit_44100.wav")
wave3 = WaveSoundResource("8bit_22050.wav")


camera = CameraNode()


while True:
    # print(engine.get_running_fps())
    
    if engine_io.RB.is_just_pressed:
        c = engine_audio.play(wave0, 0, False)
        c.gain = 1
        
    if engine_io.LB.is_just_pressed:
        c = engine_audio.play(tone0, 1, False)
        c.gain = 1
    
    if engine_io.A.is_just_pressed:
        c = engine_audio.play(wave2, 2, False)
        c.gain = 1
        
    if engine_io.B.is_just_pressed:
        c = engine_audio.play(wave3, 3, False)
        c.gain = 1
    
    if engine_io.UP.is_pressed:
        tone0.frequency += 10.0
        print(tone0.frequency)
    elif engine_io.DOWN.is_pressed:
        tone0.frequency -= 10.0
        print(tone0.frequency)
    
    engine.tick()
