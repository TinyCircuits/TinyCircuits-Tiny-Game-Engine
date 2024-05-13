import engine_main

import engine
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
from engine_resources import NoiseResource
import engine_io
import math
# import machine
# machine.freq(250 * 1000 * 1000)

engine.set_fps_limit(60)

class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.mode = 0
        self.position.z = -25

    def forward(self):
        self.position.x += math.cos(self.rotation.y) * self.distance
        self.position.z += math.sin(self.rotation.y) * self.distance
    
    def backward(self):
        self.position.x -= math.cos(self.rotation.y) * self.distance
        self.position.z -= math.sin(self.rotation.y) * self.distance
    
    def left(self):
        self.position.x -= math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z -= math.sin(self.rotation.y+(math.pi/2)) * self.distance
    
    def right(self):
        self.position.x += math.cos(self.rotation.y+(math.pi/2)) * self.distance
        self.position.z += math.sin(self.rotation.y+(math.pi/2)) * self.distance

    def tick(self, dt):
        # print(engine.get_running_fps())

        if engine_io.check_pressed(engine_io.BUMPER_RIGHT):
            self.rotation.y += 0.05
            engine_io.rumble(self.rotation.y)
        if engine_io.check_pressed(engine_io.BUMPER_LEFT):
            self.rotation.y -= 0.05
            engine_io.rumble(self.rotation.y)
    

        if engine_io.check_pressed(engine_io.DPAD_UP):
            self.forward()
        if engine_io.check_pressed(engine_io.DPAD_DOWN):
            self.backward()
        if engine_io.check_pressed(engine_io.DPAD_LEFT):
            self.left()
        if engine_io.check_pressed(engine_io.DPAD_RIGHT):
            self.right()
        
        if engine_io.check_pressed(engine_io.A):
            if self.mode == 0:
                self.position.y -= 1
            else:
                self.rotation.x -= 0.5
        if engine_io.check_pressed(engine_io.B):
            if self.mode == 0:
                self.position.y += 1
            else:
                self.rotation.x += 0.5
        
        if engine_io.check_pressed(engine_io.MENU):
            if self.mode == 0:
                self.mode = 1
            else:
                self.mode = 0


camera = MyCam()
mesh = MeshNode()
noise = NoiseResource()


def add_quad(v1, v2, v3, v4):
    mesh.vertices.append(v1)
    mesh.vertices.append(v2)
    mesh.vertices.append(v3)

    mesh.vertices.append(v1)
    mesh.vertices.append(v4)
    mesh.vertices.append(v3)


def is_solid(x, y, z):
    if noise.noise_3d(x*3, y*3, z*3) < 0.25:
        return True
    else:
        return False

size = 4

for x in range(16):
    for y in range(16):
        for z in range(16):
            gx = x * size - 8*size
            gy = y * size - 8*size
            gz = z * size - 8*size

            this_solid = is_solid(x, y, z)
            
            if this_solid != is_solid(x+1, y, z):
                add_quad(Vector3(gx, gy-size, gz-size),
                         Vector3(gx, gy,      gz-size),
                         Vector3(gx, gy,      gz),
                         Vector3(gx, gy-size, gz))
            
            if this_solid != is_solid(x, y+1, z):
                add_quad(Vector3(gx-size, gy, gz-size),
                         Vector3(gx,      gy, gz-size),
                         Vector3(gx,      gy, gz),
                         Vector3(gx-size, gy, gz))
            
            if this_solid != is_solid(x, y, z+1):
                add_quad(Vector3(gx-size, gy-size, gz),
                         Vector3(gx,      gy-size, gz),
                         Vector3(gx,      gy,      gz),
                         Vector3(gx-size, gy,      gz))




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