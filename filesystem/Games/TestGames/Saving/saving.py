import engine_main
import engine_save
from engine_math import Vector2, Vector3
from engine_draw import Color

from engine_nodes import CameraNode

engine_save.set_location("save.data")

engine_save.save("test0", "Hi there 1131!!!!")
engine_save.save("test1", "YoYo")
engine_save.save("test2", "Hello")
engine_save.save("to_delete", "I will be deleted :(")
engine_save.save("test3", 1000)
engine_save.save("test4", 0.987654321)
engine_save.save("test5", Vector2(0.1, 0.2))
engine_save.save("test6", Vector3(0.3, 0.4, 0.5))
engine_save.save("test7", Color(0.9, 0.8, 0.7))
engine_save.save("test8", bytearray([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]))


print(engine_save.load("test0", "default"))
print(engine_save.load("test1", "default"))
print(engine_save.load("test2", "default"))
print(engine_save.load("to_delete", "default"))
print(engine_save.load("test3", 0))
print(engine_save.load("test4", 0.0))
print(engine_save.load("test5", Vector2(0, 0)))
print(engine_save.load("test6", Vector3(0, 0, 0)))
print(engine_save.load("test7", Color(0, 0, 0)))

print(bytearray([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]))
print(engine_save.load("test8", bytearray(0)))


engine_save.delete("to_delete")
print(engine_save.load("to_delete", "If you're seeing this, I was deleted and this is the default value!"))