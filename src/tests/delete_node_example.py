import engine
import engine_io
from engine_nodes import Circle2DNode, CameraNode
from engine_math import Vector2
import random
import gc

# NOTE: looks like there is a bug that's not deleting the last circle, looking into it...

# Because there's no debouncing, set FPS low so it samples buttons less often...
engine.set_fps_limit(10)

# List to hold all the circles we'll be adding and deleting
circles_list = []

camera = CameraNode()

# By the way, you aren't required to use node callbacks to pack all your
# game logic into, you can also have a while loop and use `engine.tick()`
# instead of `engine.start()`. The callbacks will still be called normally
# but now you have a global scope to put overarching game code
while True:

    # Once A is pressed, delete a circle node (marks for collection by gc)
    # and then force it to be collected now instead of later
    if engine_io.A.is_pressed and len(circles_list) > 0:

        # In the future I will add a function that can be called on each node
        # like `my_node.destroy()`
        del circles_list[0]
        gc.collect()
        print("Deleted a node!")
    
    # Add a circle once B is pressed
    if engine_io.B.is_pressed:
        new_circle = Circle2DNode()
        new_circle.position.x = random.uniform(-1.0, 1.0) * 64.0
        new_circle.position.y = random.uniform(-1.0, 1.0) * 64.0
        circles_list.append(new_circle)
        print("Added a node!")

    engine.tick()
