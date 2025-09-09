import engine_main
import engine_io
import engine_link
import engine
from engine_nodes import CameraNode, Text2DNode, Rectangle2DNode
from engine_math import Vector2
import time


# Only works in bottom right of screen where y and x are +
class SyncedPlayer(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.buffer = bytearray(8) # Enough for x and y float bytes
        self.speed = 0.5

    def handle_recv(self):
        # If at least 8 bytes are available, read 8 bytes into our buffer
        # and reconstruct the position
        if engine_link.available() >= 8:
            engine_link.read_into(self.buffer, 8)

            x = 0
            y = 0

            x = x | (self.buffer[0] << 24)
            x = x | (self.buffer[1] << 16)
            x = x | (self.buffer[2] << 8)
            x = x | (self.buffer[3] << 0)

            y = y | (self.buffer[4] << 24)
            y = y | (self.buffer[5] << 16)
            y = y | (self.buffer[6] << 8)
            y = y | (self.buffer[7] << 0)

            self.position.x = x
            self.position.y = y
    
    def handle_send(self):
        should_sync = False

        if engine_io.UP.is_pressed:
            self.position.y -= self.speed
            should_sync = True
        elif engine_io.DOWN.is_pressed:
            self.position.y += self.speed
            should_sync = True
        elif engine_io.LEFT.is_pressed:
            self.position.x -= self.speed
            should_sync = True
        elif engine_io.RIGHT.is_pressed:
            self.position.x += self.speed
            should_sync = True
        
        if should_sync:
            x = int(self.position.x)
            y = int(self.position.y)

            self.buffer[0] = (x >> 24) & 0b11111111
            self.buffer[1] = (x >> 16) & 0b11111111
            self.buffer[2] = (x >> 8) & 0b11111111
            self.buffer[3] = (x >> 0) & 0b11111111

            self.buffer[4] = (y >> 24) & 0b11111111
            self.buffer[5] = (y >> 16) & 0b11111111
            self.buffer[6] = (y >> 8) & 0b11111111
            self.buffer[7] = (y >> 0) & 0b11111111

            engine_link.send(self.buffer)

    def tick(self, dt):
        self.handle_recv()
        self.handle_send()



player = SyncedPlayer()
text_node_started = Text2DNode(text="started: False", position=Vector2(0, -30))
text_node_is_host = Text2DNode(text="is host: False", position=Vector2(0, -15))
text_node = Text2DNode(text="not connected")
text_node_cb = Text2DNode(text="CB: not connected", position=Vector2(0, 15))
text_node_available = Text2DNode(text="available: ~", position=Vector2(0, 30))
camera = CameraNode()


def connected():
    text_node_cb.text = "CB: connected"

    # Clear anything that might not be related to the game
    engine_link.clear_send()
    engine_link.clear_read()

def disconnected():
    text_node_cb.text = "CB: not connected"

engine_link.start()
engine_link.set_connected_cb(connected)
engine_link.set_disconnected_cb(disconnected)

while True:
    if not engine.tick():
        continue

    text_node_started.text = "started: " + str(engine_link.is_started())

    text_node_is_host.text = "is host: " + str(engine_link.is_host())

    text_node_available.text = "available: " + str(engine_link.available())
    
    if engine_link.connected():
        text_node.text = "connected"
    else:
        text_node.text = "not connected"