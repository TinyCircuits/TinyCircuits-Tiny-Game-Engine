import engine_main

import engine
from engine_nodes import Circle2DNode, CameraNode

circle = Circle2DNode()

camera = CameraNode()

ticks = 0
ticks_end = 60 * 5
fps_total = 0
while ticks < ticks_end:
    engine.tick()
    fps_total = fps_total + engine.get_running_fps()
    ticks = ticks + 1 


print("-[circle_node_perf_test.py, avg. FPS: " + str(fps_total / ticks_end) + "]-")

import machine
machine.soft_reset()