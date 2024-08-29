import engine_main
import engine_io
import engine_link
import engine
from engine_nodes import CameraNode, Text2DNode
from engine_math import Vector2


text_node = Text2DNode(text="not connected")
text_node_cb = Text2DNode(text="", position=Vector2(0, 20))
camera = CameraNode()


def connected():
    text_node_cb.text = "CB: connected"


def disconnected():
    text_node_cb.text = "CB: not connected"


engine_link.start()
engine_link.set_connected_cb(connected)
engine_link.set_disconnected_cb(disconnected)

while True:
    if not engine.tick():
        continue

    if engine_io.LB.is_just_pressed:
        engine_link.start()
    elif engine_io.RB.is_just_pressed:
        engine_link.stop()
    
    if engine_io.UP.is_pressed:
        camera.position.y -= 0.5
    elif engine_io.DOWN.is_pressed:
        camera.position.y += 0.5
    
    if engine_link.connected():
        text_node.text = "connected"
    else:
        text_node.text = "not connected"