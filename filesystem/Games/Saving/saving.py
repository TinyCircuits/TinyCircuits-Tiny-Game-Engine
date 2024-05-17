import engine_main
import engine_save

from engine_nodes import CameraNode

cam = CameraNode()

engine_save.set_location("save.data")
engine_save.save("test", 1)