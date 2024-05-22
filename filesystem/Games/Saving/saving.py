import engine_main
import engine_save

from engine_nodes import CameraNode

cam = CameraNode()

engine_save.set_location("save.data")

engine_save.save("test0", "HI")
engine_save.save("test1", 9174987/19438198743)
engine_save.save("test2", 1000)


print(engine_save.load("test0", "default"))
print(engine_save.load("test1", 0.1))
print(engine_save.load("test2", 0))

# engine_save.delete("test1")