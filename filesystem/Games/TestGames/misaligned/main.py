import engine_main
import engine
from engine_nodes import CameraNode, Rectangle2DNode
import engine_io

camera = CameraNode()
sprite = Rectangle2DNode(width=112, height=16)

while True:
    if engine.tick():
        if engine_io.A.is_just_pressed: break #quit
        
        if engine_io.DOWN.is_just_pressed:
            sprite.position.y += 1
            print(sprite.position)
        if engine_io.UP.is_just_pressed:
            sprite.position.y -= 1
            print(sprite.position)
        if engine_io.RIGHT.is_just_pressed:
            sprite.position.x += 1
            print(sprite.position)
        if engine_io.LEFT.is_just_pressed:
            sprite.position.x -= 1
            print(sprite.position)