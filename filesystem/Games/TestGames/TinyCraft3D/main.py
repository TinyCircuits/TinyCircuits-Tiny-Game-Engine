import engine_main

import engine
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
from engine_resources import NoiseResource, MeshResource
import engine_io
import math
import engine_draw

# engine.freq(250 * 1000 * 1000)

# engine.fps_limit(60)
engine.disable_fps_limit()

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.fov = 75.0

        self.rotation.y = -90

        for i in range(100):
            self.backward()

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
        print(engine.get_running_fps())

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
mesh_resource = MeshResource()
mesh = MeshNode(mesh=mesh_resource)
noise = NoiseResource()
noise.seed = 69
noise.frequency = 0.0125

mesh.scale.x = 1.25
mesh.scale.y = 1.25
mesh.scale.z = 1.25

def add_quad(color, v1, v2, v3, v4):
    mesh_resource.vertices.append(v1)
    mesh_resource.vertices.append(v2)
    mesh_resource.vertices.append(v3)
    mesh_resource.triangle_colors.append(color)

    mesh_resource.vertices.append(v3)
    mesh_resource.vertices.append(v4)
    mesh_resource.vertices.append(v1)
    mesh_resource.triangle_colors.append(color)


def is_solid(x, y, z):
    if noise.noise_3d(x*3, y*3, z*3) < 0.25:
        return True
    else:
        return False

size = 4
chunk_size = 15

for x in range(chunk_size):
    for y in range(chunk_size):
        for z in range(chunk_size):
            gx = x * size - 8*size
            gy = y * size - 8*size
            gz = z * size - 8*size

            this_solid = is_solid(x, y, z)

            if this_solid != is_solid(x+1, y, z):
                if this_solid:
                    add_quad(engine_draw.brown,
                             Vector3(gx, gy-size, gz-size),
                             Vector3(gx, gy,      gz-size),
                             Vector3(gx, gy,      gz),
                             Vector3(gx, gy-size, gz))
                else:
                    add_quad(engine_draw.brown,
                             Vector3(gx, gy-size, gz-size),
                             Vector3(gx, gy-size, gz),
                             Vector3(gx, gy,      gz),
                             Vector3(gx, gy,      gz-size))

            if this_solid != is_solid(x, y+1, z):
                if this_solid:
                    add_quad(engine_draw.brown,
                             Vector3(gx-size, gy, gz-size),
                             Vector3(gx-size, gy, gz),
                             Vector3(gx,      gy, gz),
                             Vector3(gx,      gy, gz-size))
                else:
                    add_quad(engine_draw.green,
                             Vector3(gx-size, gy, gz-size),
                             Vector3(gx,      gy, gz-size),
                             Vector3(gx,      gy, gz),
                             Vector3(gx-size, gy, gz))

            if this_solid != is_solid(x, y, z+1):
                if this_solid:
                    add_quad(engine_draw.brown,
                             Vector3(gx-size, gy-size, gz),
                             Vector3(gx,      gy-size, gz),
                             Vector3(gx,      gy,      gz),
                             Vector3(gx-size, gy,      gz))
                else:
                    add_quad(engine_draw.brown,
                             Vector3(gx-size, gy-size, gz),
                             Vector3(gx-size, gy,      gz),
                             Vector3(gx,      gy,      gz),
                             Vector3(gx,      gy-size, gz))


# mesh.vertices.append(Vector3(-5, -5, 1))
# mesh.vertices.append(Vector3(5, -5, 1))
# mesh.vertices.append(Vector3(0, 5, 5))

# mesh.vertices.append(Vector3(-5, -5, -1))
# mesh.vertices.append(Vector3(5, -5, -1))
# mesh.vertices.append(Vector3(0, 5, -5))

# mesh.vertices.append(Vector3(-5, -5, 1))
# mesh.vertices.append(Vector3(5, -5, 1))
# mesh.vertices.append(Vector3(0, -10, 5))

# mesh.vertices.append(Vector3(-5, -5, -1))
# mesh.vertices.append(Vector3(5, -5, -1))
# mesh.vertices.append(Vector3(0, -10, -5))


# print(mesh.vertices)

engine.start()