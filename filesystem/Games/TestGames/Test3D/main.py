import engine_main

import engine
import engine_draw
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
import engine_io
import math

# engine.freq(250 * 1000 * 1000)

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
mesh = MeshNode(color=engine_draw.orange)
floor = MeshNode(color=engine_draw.red)


def add_quad(to_add_to, v1, v2, v3, v4):
    to_add_to.vertices.append(v1)
    to_add_to.vertices.append(v2)
    to_add_to.vertices.append(v3)

    to_add_to.vertices.append(v3)
    to_add_to.vertices.append(v4)
    to_add_to.vertices.append(v1)


add_quad(mesh, Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(1, 0, 0))
add_quad(mesh, Vector3(0, 1, 0), Vector3(0, 2, 0), Vector3(1, 2, 0), Vector3(1, 1, 0))
add_quad(mesh, Vector3(0, 2, 0), Vector3(0, 3, 0), Vector3(1, 3, 0), Vector3(1, 2, 0))
add_quad(mesh, Vector3(0, 3, 0), Vector3(0, 4, 0), Vector3(1, 4, 0), Vector3(1, 3, 0))
add_quad(mesh, Vector3(0, 4, 0), Vector3(0, 5, 0), Vector3(1, 5, 0), Vector3(1, 4, 0))

add_quad(mesh, Vector3(-1, 3, 0), Vector3(-1, 4, 0), Vector3(0, 4, 0), Vector3(0, 3, 0))
add_quad(mesh, Vector3(1, 3, 0), Vector3(1, 4, 0), Vector3(2, 4, 0), Vector3(2, 3, 0))

add_quad(mesh, Vector3(-2, 2, 0), Vector3(-2, 3, 0), Vector3(-1, 3, 0), Vector3(-1, 2, 0))
add_quad(mesh, Vector3(3, 2, 0), Vector3(3, 3, 0), Vector3(2, 3, 0), Vector3(2, 2, 0))


add_quad(floor, Vector3(-5, -3, -5), Vector3(5, -3, -5), Vector3(5, -3, 5), Vector3(-5, -3, 5))


t = 0


while True:
    if engine.tick() is False:
        continue

    mesh.rotation.x += 0.01
    mesh.rotation.y += 0.01
    mesh.rotation.z += 0.01

    mesh.position.x = math.sin(t*2) * 4
    mesh.position.y = math.sin(t) * 2
    mesh.position.z = math.cos(t*2) * 4

    t += 0.01