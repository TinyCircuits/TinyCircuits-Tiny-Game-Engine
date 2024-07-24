import engine_main
import engine
import engine_time
import engine_io


while True:
    if engine.tick():
        if engine_io.A.is_just_pressed:
            print("Get")
            print(engine_time.datetime())
        if engine_io.B.is_just_pressed:
            print("Set")
            engine_time.datetime((2024, 7, 24, 11, 49, 0))
