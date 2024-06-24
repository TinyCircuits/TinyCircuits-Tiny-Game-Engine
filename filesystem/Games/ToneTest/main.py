import engine_main
import engine
import engine_audio
import engine_io

from engine_resources import ToneSoundResource, WaveSoundResource

t = ToneSoundResource()
w = WaveSoundResource("rtttl.wav")

print("Done loading!")

while True:
    if engine.tick():
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            t.frequency -= 100
            print(t.frequency)
        elif engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            t.frequency += 100
            print(t.frequency)
        
        if engine_io.check_pressed(engine_io.A):
            engine_audio.play(t, 0, True)
        elif engine_io.check_pressed(engine_io.B):
            engine_audio.play(w, 0, True)