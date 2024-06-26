import engine_main
import engine
import engine_audio
import engine_io

from engine_resources import ToneSoundResource, WaveSoundResource, RTTTLSoundResource

engine_audio.set_volume(5.0)

t = ToneSoundResource()
r = RTTTLSoundResource("test.rtttl")
w0 = WaveSoundResource("rtttl.wav")
w1 = WaveSoundResource("random.wav")
w2 = WaveSoundResource("random (1).wav")
w3 = WaveSoundResource("random (2).wav")
w4 = WaveSoundResource("random (3).wav")
w5 = WaveSoundResource("rtttl_blasted.wav")

wavs = [r, w0, w1, w2, w3, w4, w5]

print("Done loading!")

c = 0

while True:
    if engine.tick():
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            t.frequency -= 100
            print(t.frequency)
        elif engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            t.frequency += 100
            print(t.frequency)
        
        if engine_io.check_just_pressed(engine_io.A):
            engine_audio.play(t, 0, True)
        elif engine_io.check_just_pressed(engine_io.B):
            print(c)
            engine_audio.play(wavs[c], 0, False)
            c = c + 1

            if c > 6:
                c = 0