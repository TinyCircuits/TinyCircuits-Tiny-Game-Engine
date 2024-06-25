import engine
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
import engine_io
import math

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

    def tick(self):
        print(self.position)

        if engine_io.RB.is_pressed:
            self.rotation.y += 0.05
            engine_io.rumble(self.rotation.y)
        if engine_io.LB.is_pressed:
            self.rotation.y -= 0.05
            engine_io.rumble(self.rotation.y)
    

        if engine_io.UP.is_pressed:
            self.forward()
        if engine_io.DOWN.is_pressed:
            self.backward()
        if engine_io.LEFT.is_pressed:
            self.left()
        if engine_io.RIGHT.is_pressed:
            self.right()
        
        if engine_io.A.is_pressed:
            if self.mode == 0:
                self.position.y -= 1
            else:
                self.rotation.x -= 0.5
        if engine_io.B.is_pressed:
            if self.mode == 0:
                self.position.y += 1
            else:
                self.rotation.x += 0.5
        
        if engine_io.MENU.is_pressed:
            if self.mode == 0:
                self.mode = 1
            else:
                self.mode = 0


camera = MyCam()
mesh = MeshNode()

mesh.vertices.append(Vector3(-5, -5, 1))
mesh.vertices.append(Vector3(5, -5, 1))
mesh.vertices.append(Vector3(0, 5, 5))

mesh.vertices.append(Vector3(-5, -5, -1))
mesh.vertices.append(Vector3(5, -5, -1))
mesh.vertices.append(Vector3(0, 5, -5))

mesh.vertices.append(Vector3(-5, -5, 1))
mesh.vertices.append(Vector3(5, -5, 1))
mesh.vertices.append(Vector3(0, -10, 5))

mesh.vertices.append(Vector3(-5, -5, -1))
mesh.vertices.append(Vector3(5, -5, -1))
mesh.vertices.append(Vector3(0, -10, -5))


print(mesh.vertices)

engine.start()