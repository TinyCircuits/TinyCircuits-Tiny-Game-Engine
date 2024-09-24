import engine_main
import engine_io
import engine_link
import engine
from engine_nodes import CameraNode, Text2DNode, Rectangle2DNode
import time

# Disable so that every `engine.tick()` call renders the scene right away
engine.disable_fps_limit()

# Render the scene once
text_node = Text2DNode(text="not connected")
camera = CameraNode()
engine.tick()

# Start link discovery (lets Thumby act as a host sometimes)
engine_link.start()

# Wait for connection
while True:
    if engine_link.connected():
        text_node.text = "connected"
        break

# Render the scene one more time
engine.tick()

# Pause forever so we can see "connected"
while True:
    pass