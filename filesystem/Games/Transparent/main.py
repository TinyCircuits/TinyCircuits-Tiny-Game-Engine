import engine_main

import random

import engine
import engine_io
import engine_physics
import engine_draw
from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, PhysicsRectangle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_math import Vector2
import os


engine.set_fps_limit(60)
engine_draw.set_background_color(engine_draw.white)


texture = TextureResource("PlayerTile_16bit.bmp")
test = Sprite2DNode(texture=texture, transparent_color=Color(0b0000011111100000))

cam = CameraNode()

engine.start()