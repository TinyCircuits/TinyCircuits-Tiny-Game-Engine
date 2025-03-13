import engine_main
import engine
from engine_nodes import Text2DNode, CameraNode

cam = CameraNode()
text = Text2DNode(text="KAKAKAKAKAKAKAKA", letter_spacing=1.0)

engine.start()