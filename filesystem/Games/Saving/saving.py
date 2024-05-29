import engine_main
import engine_save
from engine_math import Vector2, Vector3
from engine_draw import Color

from engine_nodes import CameraNode

engine_save.set_location("save.data")

engine_save.save("test0", "Hi there!!!!")
# engine_save.save("test1", 9174987.0/19438198743.0)
# engine_save.save("test2", 1000)
# engine_save.save("test3", Vector2(0.1, 0.2))
# engine_save.save("test4", Vector3(0.3, 0.4, 0.5))
# engine_save.save("test5", Color(0.9, 0.8, 0.7))
# engine_save.save("test6", bytearray([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]))


# print(engine_save.load("test0", "default"))
# print(engine_save.load("test1", 0.1))
# print(engine_save.load("test2", 0))
# print(engine_save.load("test3", Vector2(0, 0)))
# print(engine_save.load("test4", Vector3(0, 0, 0)))
# print(engine_save.load("test5", Color(0, 0, 0)))
# print(engine_save.load("test6", bytearray(0)))


# test0 = "aaaaaa"
# engine_save.load_into("test0", test0, 3)
# print(test0)

# test1 = 0.0
# engine_save.load_into("test1", test1)
# print(test1)

# test3 = Vector2(0.0, 0.0)
# engine_save.load_into("test3", test3)
# print(test3)

# test4 = Vector3(0.0, 0.0, 0.0)
# engine_save.load_into("test4", test4)
# print(test4)

# test5 = Color(0.0, 0.0, 0.0)
# engine_save.load_into("test5", test5)
# print(test5)

# test6 = bytearray(15)
# engine_save.load_into("test6", test6)
# print(test6)

# engine_save.delete("test1")