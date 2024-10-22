import engine_main

import engine
import engine_draw
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector2, Vector3, Matrix4x4
import engine_io
import math
from engine_resources import MeshResource, TextureResource


engine.freq(250 * 1000 * 1000)

engine.fps_limit(60)

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.fov = 100.0
        self.rotate_type = 0

        # self.rotation.y = -90

        # for i in range(50):
        #     self.backward()

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

    def tick(self, dt):
        # print(self.position)
        # print(engine.get_running_fps())

        if engine_io.MENU.is_just_pressed:
            self.rotate_type += 1

            if self.rotate_type >= 3:
                self.rotate_type = 0
            
            print("Rotate type:", self.rotate_type)

        if engine_io.RB.is_pressed:
            if self.rotate_type == 0:
                self.rotation.y -= 0.05
                print(self.rotation)
            elif self.rotate_type == 1:
                self.rotation.x -= 0.05
                print(self.rotation)
            elif self.rotate_type == 2:
                self.rotation.z -= 0.05
                print(self.rotation)
        if engine_io.LB.is_pressed:
            if self.rotate_type == 0:
                self.rotation.y += 0.05
                print(self.rotation)
            elif self.rotate_type == 1:
                self.rotation.x += 0.05
                print(self.rotation)
            elif self.rotate_type == 2:
                self.rotation.z += 0.05
                print(self.rotation)


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
camera.position = Vector3(0, 5, 25)

checker_texture = TextureResource("Games/TestGames/OBJ3D_TEST/checker.bmp")
obj_mesh = MeshResource("Games/TestGames/OBJ3D_TEST/cube.obj")
obj_mesh_node = MeshNode(mesh=obj_mesh, color=engine_draw.orange, texture=checker_texture)


while True:
    if engine.tick() is False:
        continue