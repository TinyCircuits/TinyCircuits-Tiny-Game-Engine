import engine_main

import engine
from engine_nodes import CameraNode, MeshNode
from engine_math import Vector3, Matrix4x4
from engine_resources import NoiseResource, MeshResource, TextureResource
import engine_io
import math
import engine_draw


engine.freq(250 * 1000 * 1000)

# engine.fps_limit(60)
engine.disable_fps_limit()

texture = TextureResource("Games/TestGames/TinyCraft3D/atlas.bmp")

CHUNK_SIZE_VOXELS = 10
VOXEL_SIZE = 5


noise = NoiseResource()
noise.seed = 69
noise.frequency = 0.05


class Chunk(MeshNode):
    def __init__(self):
        super().__init__(self)

        self.texture = texture

        # Allocate enough 8-bit xyz vertex memory for worst case lattice chunk
        worst_case_voxel_count = (CHUNK_SIZE_VOXELS*CHUNK_SIZE_VOXELS*CHUNK_SIZE_VOXELS)//2
        vertices_per_voxel_face = 6
        faces_per_voxel = 6
        bytes_per_8bit_vertex = 3

        tris_per_face = 2
        tris_per_voxel = tris_per_face*faces_per_voxel
        tri_colors_per_voxel = tris_per_voxel
        worst_case_tri_color_count = tri_colors_per_voxel * worst_case_voxel_count
        worst_case_tri_color_byte_count = worst_case_tri_color_count*2

        worst_case_vertex_count = worst_case_voxel_count * vertices_per_voxel_face * faces_per_voxel
        worst_case_vertex_byte_count = worst_case_vertex_count * bytes_per_8bit_vertex

        uvs_per_voxel = vertices_per_voxel_face * faces_per_voxel
        worst_case_uv_byte_count = worst_case_voxel_count * uvs_per_voxel * 2

        print("Worst case chunk byte count:", worst_case_vertex_byte_count, worst_case_tri_color_byte_count, worst_case_vertex_byte_count+worst_case_tri_color_byte_count)

        self.mesh = MeshResource(bytearray(worst_case_vertex_byte_count), [], bytearray(worst_case_uv_byte_count), bytearray(worst_case_tri_color_byte_count))
    
    def is_solid(self, x, y, z):
        if noise.noise_3d(self.cx+x, self.cy+y, self.cz+z) < 0.25:
        # if self.cy+y+noise.noise_2d(self.cx+x, self.cz+z)*2 > 1:
            return True
        else:
            return False

    def add_quad(self, color, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z,  v1u, v1v,  v2u, v2v,  v3u, v3v,  v4u, v4v):
        vertex_byte_offset = self.mesh.vertex_count*3
        uv_byte_offset = self.mesh.vertex_count*2

        self.mesh.vertices[vertex_byte_offset] = v1x
        self.mesh.vertices[vertex_byte_offset+1] = v1y
        self.mesh.vertices[vertex_byte_offset+2] = v1z
        self.mesh.uvs[uv_byte_offset] = v1u
        self.mesh.uvs[uv_byte_offset+1] = v1v

        self.mesh.vertices[vertex_byte_offset+6] = v3x
        self.mesh.vertices[vertex_byte_offset+7] = v3y
        self.mesh.vertices[vertex_byte_offset+8] = v3z
        self.mesh.uvs[uv_byte_offset+2] = v3u
        self.mesh.uvs[uv_byte_offset+3] = v3v

        self.mesh.vertices[vertex_byte_offset+3] = v2x
        self.mesh.vertices[vertex_byte_offset+4] = v2y
        self.mesh.vertices[vertex_byte_offset+5] = v2z
        self.mesh.uvs[uv_byte_offset+4] = v2u
        self.mesh.uvs[uv_byte_offset+5] = v2v


        self.mesh.vertices[vertex_byte_offset+9] = v3x
        self.mesh.vertices[vertex_byte_offset+10] = v3y
        self.mesh.vertices[vertex_byte_offset+11] = v3z
        self.mesh.uvs[uv_byte_offset+6] = v3u
        self.mesh.uvs[uv_byte_offset+7] = v3v

        self.mesh.vertices[vertex_byte_offset+15] = v1x
        self.mesh.vertices[vertex_byte_offset+16] = v1y
        self.mesh.vertices[vertex_byte_offset+17] = v1z
        self.mesh.uvs[uv_byte_offset+8] = v1u
        self.mesh.uvs[uv_byte_offset+9] = v1v

        self.mesh.vertices[vertex_byte_offset+12] = v4x
        self.mesh.vertices[vertex_byte_offset+13] = v4y
        self.mesh.vertices[vertex_byte_offset+14] = v4z
        self.mesh.uvs[uv_byte_offset+10] = v4u
        self.mesh.uvs[uv_byte_offset+11] = v4v


        triangle_index = self.mesh.vertex_count//3
        triangle_color_byte_offset = triangle_index*2

        self.mesh.triangle_colors[triangle_color_byte_offset] = (color.value >> 0) & 0b11111111
        self.mesh.triangle_colors[triangle_color_byte_offset+1] = (color.value >> 8) & 0b11111111

        self.mesh.triangle_colors[triangle_color_byte_offset+2] = (color.value >> 0) & 0b11111111
        self.mesh.triangle_colors[triangle_color_byte_offset+3] = (color.value >> 8) & 0b11111111

        self.mesh.vertex_count += 6

    def generate(self, cx, cy, cz):
        self.cx = cx * CHUNK_SIZE_VOXELS
        self.cy = cy * CHUNK_SIZE_VOXELS
        self.cz = cz * CHUNK_SIZE_VOXELS

        self.position.x = cx * VOXEL_SIZE * CHUNK_SIZE_VOXELS
        self.position.y = cy * VOXEL_SIZE * CHUNK_SIZE_VOXELS
        self.position.z = cz * VOXEL_SIZE * CHUNK_SIZE_VOXELS

        # print("Chunk:", self.position)

        self.mesh.vertex_count = 0

        for x in range(CHUNK_SIZE_VOXELS):
            for y in range(CHUNK_SIZE_VOXELS):
                for z in range(CHUNK_SIZE_VOXELS):
                    gx = x * VOXEL_SIZE - (CHUNK_SIZE_VOXELS*VOXEL_SIZE//2)
                    gy = y * VOXEL_SIZE - (CHUNK_SIZE_VOXELS*VOXEL_SIZE//2)
                    gz = z * VOXEL_SIZE - (CHUNK_SIZE_VOXELS*VOXEL_SIZE//2)

                    this_solid = self.is_solid(x, y, z)

                    if this_solid != self.is_solid(x+1, y, z):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx, gy-VOXEL_SIZE, gz-VOXEL_SIZE,
                                    gx, gy,            gz-VOXEL_SIZE,
                                    gx, gy,            gz,
                                    gx, gy-VOXEL_SIZE, gz,
                                    0, 0,
                                    64, 0,
                                    64, 64,
                                    0, 64)
                        else:
                            self.add_quad(engine_draw.brown,
                                    gx, gy-VOXEL_SIZE, gz-VOXEL_SIZE,
                                    gx, gy-VOXEL_SIZE, gz,
                                    gx, gy,            gz,
                                    gx, gy,            gz-VOXEL_SIZE,
                                    0, 0,
                                    64, 0,
                                    64, 64,
                                    0, 64)

                    if this_solid != self.is_solid(x, y+1, z):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy, gz-VOXEL_SIZE,
                                    gx-VOXEL_SIZE, gy, gz,
                                    gx,            gy, gz,
                                    gx,            gy, gz-VOXEL_SIZE,
                                    0, 0,
                                    64, 0,
                                    64, 64,
                                    0, 64)
                        else:
                            self.add_quad(engine_draw.green,
                                    gx-VOXEL_SIZE, gy, gz-VOXEL_SIZE,
                                    gx,            gy, gz-VOXEL_SIZE,
                                    gx,            gy, gz,
                                    gx-VOXEL_SIZE, gy, gz,
                                    128-1, 0,
                                    192-1, 0,
                                    192-1, 64-1,
                                    128-1, 64-1)

                    if this_solid != self.is_solid(x, y, z+1):
                        if this_solid:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy-VOXEL_SIZE, gz,
                                    gx,            gy-VOXEL_SIZE, gz,
                                    gx,            gy,            gz,
                                    gx-VOXEL_SIZE, gy,            gz,
                                    0, 0,
                                    64-1, 0,
                                    64-1, 64-1,
                                    0, 64-1)
                        else:
                            self.add_quad(engine_draw.brown,
                                    gx-VOXEL_SIZE, gy-VOXEL_SIZE, gz,
                                    gx-VOXEL_SIZE, gy,            gz,
                                    gx,            gy,            gz,
                                    gx,            gy-VOXEL_SIZE, gz,
                                    0, 0,
                                    64, 0,
                                    64, 64,
                                    0, 64)
        



class ChunkManager():
    def __init__(self, initial_xi, initial_yi, initial_zi):
        self.chunks = []

        for xi in range(initial_xi-1, initial_xi+3-1):
            for zi in range(initial_zi-1, initial_zi+3-1):
                chunk = Chunk()
                chunk.generate(xi, initial_yi, zi)
                self.chunks.append(chunk)
        
        self.last_xi = initial_xi
        self.last_yi = initial_yi
        self.last_zi = initial_zi
    
    def generate(self, xi, yi, zi):
        for x in range(3):
            for z in range(3):
                i = z*3 + x
                self.chunks[i].generate(xi+x, yi, zi+z)
    
    def update(self, abs_x, abs_y, abs_z):
        xi = int(abs_x) // (CHUNK_SIZE_VOXELS*VOXEL_SIZE)
        yi = int(abs_y) // (CHUNK_SIZE_VOXELS*VOXEL_SIZE)
        zi = int(abs_z) // (CHUNK_SIZE_VOXELS*VOXEL_SIZE)

        if xi != self.last_xi:
            self.last_xi = xi
            # self.chunks[0].generate(xi, yi, zi)
            self.generate(xi, yi, zi)
            return True
        elif yi != self.last_yi:
            self.last_yi = yi
            # self.chunks[0].generate(xi, yi, zi)
            self.generate(xi, yi, zi)
            return True
        elif zi != self.last_zi:
            self.last_zi = zi
            # self.chunks[0].generate(xi, yi, zi)
            self.generate(xi, yi, zi)
            return True

        return False
        

# chunk_manager = ChunkManager(0, 0, 0)

chunks = []

for x in range(2):
    for y in range(4):
        for z in range(2):
            chunk = Chunk()
            chunk.generate(x, y, z)
            chunks.append(chunk)


# for x in range(7):
#     chunk = Chunk()
#     chunk.generate(x, 0, 0)
#     chunks.append(chunk)

# chunk = Chunk()
# chunk.generate(0, 0, 0)
# chunks.append(chunk)

# chunk = Chunk()
# chunk.generate(1, 0, 0)
# chunks.append(chunk)


class MyCam(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.distance = 0.75
        self.fov = 100.0

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
        
        
        # if chunk_manager.update(self.position.x, self.position.y, self.position.z):
        #     print("Camera:", self.position)

        # chunk_manager.update(self.position.x, self.position.y, self.position.z)


camera = MyCam()


engine.start()