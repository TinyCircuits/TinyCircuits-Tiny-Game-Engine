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


CHUNK_SIZE_VOXELS = 6
VOXEL_SIZE = 4


noise = NoiseResource()
noise.seed = 69
noise.frequency = 0.0125


class Chunk(MeshNode):
    def __init__(self):
        super().__init__(self)
        # Allocate enough 8-bit xyz vertex memory for worst case lattice chunk

        worst_case_voxel_count = CHUNK_SIZE_VOXELS*CHUNK_SIZE_VOXELS*CHUNK_SIZE_VOXELS
        vertices_per_voxel_face = 6
        faces_per_voxel = 6
        bytes_per_8bit_vertex = 3

        worst_case_vertex_count = worst_case_voxel_count * vertices_per_voxel_face * faces_per_voxel
        worst_case_vertex_byte_count = worst_case_vertex_count * bytes_per_8bit_vertex

        print("Worst case chunk vertex byte count:", worst_case_vertex_byte_count)

        self.mesh = MeshResource(bytearray(worst_case_vertex_byte_count))
    
    def is_solid(self, x, y, z):
        if noise.noise_3d(x*3, y*3, z*3) < 0.25:
            return True
        else:
            return False

    def add_quad(self, color, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z):
        byte_offset = self.mesh.vertex_count*3

        self.mesh.vertices[byte_offset] = v1x
        self.mesh.vertices[byte_offset+1] = v1y
        self.mesh.vertices[byte_offset+2] = v1z

        self.mesh.vertices[byte_offset+3] = v2x
        self.mesh.vertices[byte_offset+4] = v2y
        self.mesh.vertices[byte_offset+5] = v2z

        self.mesh.vertices[byte_offset+6] = v3x
        self.mesh.vertices[byte_offset+7] = v3y
        self.mesh.vertices[byte_offset+8] = v3z


        self.mesh.vertices[byte_offset+9] = v3x
        self.mesh.vertices[byte_offset+10] = v3y
        self.mesh.vertices[byte_offset+11] = v3z

        self.mesh.vertices[byte_offset+12] = v4x
        self.mesh.vertices[byte_offset+13] = v4y
        self.mesh.vertices[byte_offset+14] = v4z

        self.mesh.vertices[byte_offset+15] = v1x
        self.mesh.vertices[byte_offset+16] = v1y
        self.mesh.vertices[byte_offset+17] = v1z

        self.mesh.vertex_count += 6

    def generate(self):
        self.mesh.vertex_count = 0

        for x in range(CHUNK_SIZE_VOXELS):
            for y in range(CHUNK_SIZE_VOXELS):
                for z in range(CHUNK_SIZE_VOXELS):
                    gx = x * VOXEL_SIZE
                    gy = y * VOXEL_SIZE
                    gz = z * VOXEL_SIZE

                    this_solid = self.is_solid(x, y, z)

                    if this_solid != self.is_solid(x+1, y, z):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx, gy-VOXEL_SIZE, gz-VOXEL_SIZE,
                                    gx, gy,            gz-VOXEL_SIZE,
                                    gx, gy,            gz,
                                    gx, gy-VOXEL_SIZE, gz)
                        else:
                            self.add_quad(engine_draw.brown,
                                    gx, gy-VOXEL_SIZE, gz-VOXEL_SIZE,
                                    gx, gy-VOXEL_SIZE, gz,
                                    gx, gy,            gz,
                                    gx, gy,            gz-VOXEL_SIZE)

                    if this_solid != self.is_solid(x, y+1, z):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy, gz-VOXEL_SIZE,
                                    gx-VOXEL_SIZE, gy, gz,
                                    gx,            gy, gz,
                                    gx,            gy, gz-VOXEL_SIZE)
                        else:
                            self.add_quad(engine_draw.green,
                                    gx-VOXEL_SIZE, gy, gz-VOXEL_SIZE,
                                    gx,            gy, gz-VOXEL_SIZE,
                                    gx,            gy, gz,
                                    gx-VOXEL_SIZE, gy, gz)

                    if this_solid != self.is_solid(x, y, z+1):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy-VOXEL_SIZE, gz,
                                    gx,            gy-VOXEL_SIZE, gz,
                                    gx,            gy,            gz,
                                    gx-VOXEL_SIZE, gy,            gz)
                        else:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy-VOXEL_SIZE, gz,
                                    gx-VOXEL_SIZE, gy,            gz,
                                    gx,            gy,            gz,
                                    gx,            gy-VOXEL_SIZE, gz)
        


# chunks = []

# for i in range(9):
#     chunks.append(Chunk())

chunk = Chunk()
chunk.generate()


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.fov = 75.0

        self.rotation.y = -90

        for i in range(50):
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
        # print(engine.get_running_fps())

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


engine.start()