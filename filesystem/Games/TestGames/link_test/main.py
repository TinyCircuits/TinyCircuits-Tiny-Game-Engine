import engine_main
import engine_io
import engine_link
import engine
from engine_nodes import CameraNode, Text2DNode


text_node = Text2DNode(text="")
camera = CameraNode()


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