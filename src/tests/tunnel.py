import engine

from engine_nodes import TunnelNode, CameraNode
from engine_resources import TextureResource

texture = TextureResource("32x32.bmp")

tunnel = TunnelNode(texture=texture)

camera = CameraNode()

engine.start()