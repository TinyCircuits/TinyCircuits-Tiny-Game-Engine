import engine_main

import engine
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
mesh = MeshNode()


def add_quad(v1, v2, v3, v4):
    mesh.vertices.append(v1)
    mesh.vertices.append(v2)
    mesh.vertices.append(v3)

    mesh.vertices.append(v3)
    mesh.vertices.append(v4)
    mesh.vertices.append(v1)


add_quad(Vector3(0, 0, -10), Vector3(0, 1, -10), Vector3(1, 1, -10), Vector3(1, 0, -10))
add_quad(Vector3(0, 1, -10), Vector3(0, 2, -10), Vector3(1, 2, -10), Vector3(1, 1, -10))
add_quad(Vector3(0, 2, -10), Vector3(0, 3, -10), Vector3(1, 3, -10), Vector3(1, 2, -10))
add_quad(Vector3(0, 3, -10), Vector3(0, 4, -10), Vector3(1, 4, -10), Vector3(1, 3, -10))
add_quad(Vector3(0, 4, -10), Vector3(0, 5, -10), Vector3(1, 5, -10), Vector3(1, 4, -10))

add_quad(Vector3(-1, 3, -10), Vector3(-1, 4, -10), Vector3(0, 4, -10), Vector3(0, 3, -10))
add_quad(Vector3(1, 3, -10), Vector3(1, 4, -10), Vector3(2, 4, -10), Vector3(2, 3, -10))

add_quad(Vector3(-2, 2, -10), Vector3(-2, 3, -10), Vector3(-1, 3, -10), Vector3(-1, 2, -10))
add_quad(Vector3(3, 2, -10), Vector3(3, 3, -10), Vector3(2, 3, -10), Vector3(2, 2, -10))


while True:
    if engine.tick() is False:
        continue

    mesh.rotation.y += 0.01