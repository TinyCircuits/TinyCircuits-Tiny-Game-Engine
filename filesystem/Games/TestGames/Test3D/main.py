import engine_main

import engine
import engine_draw
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
import engine_io
import math
from engine_resources import MeshResource


engine.freq(250 * 1000 * 1000)

engine.fps_limit(60)

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.fov = 45.0

    def forward(self):
        self.position.x -= math.sin(self.rotation.y) * self.distance
        self.position.z -= math.cos(self.rotation.y) * self.distance

    def backward(self):
        self.position.x += math.sin(self.rotation.y) * self.distance
        self.position.z += math.cos(self.rotation.y) * self.distance

    def left(self):
        self.position.x -= math.sin(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z -= math.cos(self.rotation.y+(math.pi/2)) * self.distance

    def right(self):
        self.position.x += math.sin(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z += math.cos(self.rotation.y+(math.pi/2)) * self.distance

    # def forward(self):
    #     self.position.z -= 0.1

    # def backward(self):
    #     self.position.z += 0.1

    # def left(self):
    #     self.position.x -= 0.1

    # def right(self):
    #     self.position.x += 0.1

    def tick(self, dt):
        # print(self.position)

        if engine_io.RB.is_pressed:
            self.rotation.y -= 0.05
        if engine_io.LB.is_pressed:
            self.rotation.y += 0.05


        if engine_io.UP.is_pressed:
            self.forward()
        if engine_io.DOWN.is_pressed:
            self.backward()
        if engine_io.LEFT.is_pressed:
            self.left()
        if engine_io.RIGHT.is_pressed:
            self.right()

        if engine_io.A.is_pressed:
            self.position.y += 1
        if engine_io.B.is_pressed:
            self.position.y -= 1


camera = MyCam()

arrow_mesh = MeshResource()
floor_mesh = MeshResource()


arrow_mesh_node = MeshNode(mesh=arrow_mesh, color=engine_draw.orange)
floor_mesh_node = MeshNode(mesh=floor_mesh, color=engine_draw.red)


def add_quad(to_add_to, v1, v2, v3, v4):
    to_add_to.vertices.append(v1)
    to_add_to.vertices.append(v2)
    to_add_to.vertices.append(v3)

    to_add_to.vertices.append(v3)
    to_add_to.vertices.append(v4)
    to_add_to.vertices.append(v1)


add_quad(arrow_mesh, Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(1, 0, 0))
add_quad(arrow_mesh, Vector3(0, 1, 0), Vector3(0, 2, 0), Vector3(1, 2, 0), Vector3(1, 1, 0))
add_quad(arrow_mesh, Vector3(0, 2, 0), Vector3(0, 3, 0), Vector3(1, 3, 0), Vector3(1, 2, 0))
add_quad(arrow_mesh, Vector3(0, 3, 0), Vector3(0, 4, 0), Vector3(1, 4, 0), Vector3(1, 3, 0))
add_quad(arrow_mesh, Vector3(0, 4, 0), Vector3(0, 5, 0), Vector3(1, 5, 0), Vector3(1, 4, 0))

add_quad(arrow_mesh, Vector3(-1, 3, 0), Vector3(-1, 4, 0), Vector3(0, 4, 0), Vector3(0, 3, 0))
add_quad(arrow_mesh, Vector3(1, 3, 0), Vector3(1, 4, 0), Vector3(2, 4, 0), Vector3(2, 3, 0))

add_quad(arrow_mesh, Vector3(-2, 2, 0), Vector3(-2, 3, 0), Vector3(-1, 3, 0), Vector3(-1, 2, 0))
add_quad(arrow_mesh, Vector3(3, 2, 0), Vector3(2, 2, 0), Vector3(2, 3, 0), Vector3(3, 3, 0))


add_quad(floor_mesh, Vector3(-10, -3, -10), Vector3(10, -3, -10), Vector3(10, -3, 10), Vector3(-10, -3, 10))

t = 0

while True:
    if engine.tick() is False:
        continue

    arrow_mesh_node.rotation.x += 0.005
    arrow_mesh_node.rotation.y += 0.005
    arrow_mesh_node.rotation.z += 0.005

    arrow_mesh_node.position.x = math.sin(t*2) * 4
    arrow_mesh_node.position.y = math.sin(t) * 7
    arrow_mesh_node.position.z = math.cos(t*2) * 4

    t += 0.01