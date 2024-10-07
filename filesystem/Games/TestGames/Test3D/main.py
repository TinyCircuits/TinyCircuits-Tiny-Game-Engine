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

arrow_texture = TextureResource("Games/TestGames/Test3D/cw.bmp")
floor_texture = TextureResource("Games/TestGames/Test3D/checker.bmp")

arrow_mesh = MeshResource()
floor_mesh = MeshResource()


arrow_mesh_node = MeshNode(mesh=arrow_mesh, color=engine_draw.orange, texture=arrow_texture)
floor_mesh_node = MeshNode(mesh=floor_mesh, color=engine_draw.red, texture=arrow_texture)


def add_quad(to_add_to, v1, v2, v3, v4, v1uv, v2uv, v3uv, v4uv):
    to_add_to.vertices.append(v1)
    to_add_to.uvs.append(v1uv)

    to_add_to.vertices.append(v3)
    to_add_to.uvs.append(v3uv)

    to_add_to.vertices.append(v2)
    to_add_to.uvs.append(v2uv)

    


    to_add_to.vertices.append(v3)
    to_add_to.uvs.append(v3uv)

    to_add_to.vertices.append(v1)
    to_add_to.uvs.append(v1uv)

    to_add_to.vertices.append(v4)
    to_add_to.uvs.append(v4uv)



# add_quad(arrow_mesh, Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(1, 0, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(0, 1, 0), Vector3(0, 2, 0), Vector3(1, 2, 0), Vector3(1, 1, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(0, 2, 0), Vector3(0, 3, 0), Vector3(1, 3, 0), Vector3(1, 2, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(0, 3, 0), Vector3(0, 4, 0), Vector3(1, 4, 0), Vector3(1, 3, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(0, 4, 0), Vector3(0, 5, 0), Vector3(1, 5, 0), Vector3(1, 4, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))

# add_quad(arrow_mesh, Vector3(-1, 3, 0), Vector3(-1, 4, 0), Vector3(0, 4, 0), Vector3(0, 3, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(1, 3, 0), Vector3(1, 4, 0), Vector3(2, 4, 0), Vector3(2, 3, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))

# add_quad(arrow_mesh, Vector3(-2, 2, 0), Vector3(-2, 3, 0), Vector3(-1, 3, 0), Vector3(-1, 2, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# add_quad(arrow_mesh, Vector3(3, 2, 0), Vector3(2, 2, 0), Vector3(2, 3, 0), Vector3(3, 3, 0),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))


# add_quad(floor_mesh, Vector3(-10, -3, -10), Vector3(10, -3, -10), Vector3(10, -3, 10), Vector3(-10, -3, 10),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
add_quad(floor_mesh, Vector3(-10, 0, -10), Vector3(-10, 0, 10), Vector3(10, 0, 10), Vector3(10, 0, -10),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
add_quad(floor_mesh, Vector3(-10, 0, -10), Vector3(10, 0, -10), Vector3(10, 20, -10), Vector3(-10, 20, -10), Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))

t = 0

while True:
    if engine.tick() is False:
        continue

    # arrow_mesh_node.rotation.x += 0.005
    arrow_mesh_node.rotation.y += 0.01
    # arrow_mesh_node.rotation.z += 0.005

    arrow_mesh_node.position.x = math.sin(t*2) * 4
    # arrow_mesh_node.position.y = math.sin(t) * 5
    arrow_mesh_node.position.z = math.cos(t*2) * 4

    t += 0.01



# This is gdscript code meant to be attached to a Camera3D in Godot 4.2.2 so that the above engine code
# that generates a mesh can be compared
# extends Node3D

# extends Camera3D

# # https://www.reddit.com/r/godot/comments/jh9yx7/godot_create_a_mesh_programmatically/

# var distance = 0.75
# var rotate_type = 0

# func add_quad(index:int, vert_data:PackedVector3Array, index_data:PackedInt32Array, uv_data:PackedVector2Array,
# 			  v1:Vector3, v2:Vector3, v3:Vector3, v4:Vector3,
# 			  v1uv:Vector2, v2uv:Vector2, v3uv:Vector2, v4uv:Vector2):
	
# 	vert_data.append(v1)
# 	index_data.append(index)
# 	uv_data.append(v1uv)
	
# 	vert_data.append(v3)
# 	index_data.append(index+1)
# 	uv_data.append(v3uv)
	
# 	vert_data.append(v2)
# 	index_data.append(index+2)
# 	uv_data.append(v2uv)
	
	
# 	vert_data.append(v3)
# 	index_data.append(index+3)
# 	uv_data.append(v3uv)
	
# 	vert_data.append(v1)
# 	index_data.append(index+4)
# 	uv_data.append(v1uv)
	
# 	vert_data.append(v4)
# 	index_data.append(index+5)
# 	uv_data.append(v4uv)
	
# 	index += 6
	
# 	return index


# # Called when the node enters the scene tree for the first time.
# func _ready():
# 	var texture = ResourceLoader.load("res://cw.bmp")
	
# 	var material = StandardMaterial3D.new()
# 	material.flags_unshaded = true
# 	material.albedo_texture = texture
	
# 	var light = DirectionalLight3D.new()
# 	add_child(light)
	
# 	translate(Vector3(0, 5, 25))
	
# 	var mesh = MeshInstance3D.new()
# 	var arr_mesh = ArrayMesh.new()
# 	var arr = []
# 	arr.resize(Mesh.ARRAY_MAX)

# 	var vert_data = PackedVector3Array()
# 	var index_data = PackedInt32Array()
# 	var uv_data = PackedVector2Array()
	
# 	var index:int = 0
	
# 	index = add_quad(index, vert_data, index_data, uv_data, Vector3(-10, 0, -10), Vector3(-10, 0, 10), Vector3(10, 0, 10), Vector3(10, 0, -10),  Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))
# 	index = add_quad(index, vert_data, index_data, uv_data, Vector3(-10, 0, -10), Vector3(10, 0, -10), Vector3(10, 20, -10), Vector3(-10, 20, -10), Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0))

# 	arr[Mesh.ARRAY_VERTEX]=vert_data
# 	arr[Mesh.ARRAY_TEX_UV]=uv_data
# 	arr[Mesh.ARRAY_INDEX]=index_data

# 	arr_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES,arr) #this adds a surface to the array mesh based off of array data

# 	mesh.set_mesh(arr_mesh)
# 	mesh.set_surface_override_material(0, material)
	
# 	get_parent_node_3d().add_child.call_deferred(mesh)


# # Called every frame. 'delta' is the elapsed time since the previous frame.
# func _process(_delta):
# 	if Input.is_action_just_pressed("MENU"):
# 			rotate_type += 1

# 			if rotate_type >= 3:
# 				rotate_type = 0
			
# 			print("Rotate type:", self.rotate_type)
	
# 	if Input.is_action_pressed("RB"):
# 		if rotate_type == 0:
# 			rotation.y -= 0.05
# 			print(rotation)
# 		elif rotate_type == 1:
# 			rotation.x -= 0.05
# 			print(rotation)
# 		elif rotate_type == 2:
# 			rotation.z -= 0.05
# 			print(rotation)
# 	if Input.is_action_pressed("LB"):
# 		if rotate_type == 0:
# 			rotation.y += 0.05
# 			print(rotation)
# 		elif rotate_type == 1:
# 			rotation.x += 0.05
# 			print(rotation)
# 		elif rotate_type == 2:
# 			rotation.z += 0.05
# 			print(rotation)
	
# 	if Input.is_action_pressed("UP"):
# 		position.x -= sin(rotation.y) * distance
# 		position.z -= cos(rotation.y) * distance
# 	if Input.is_action_pressed("DOWN"):
# 		position.x += sin(rotation.y) * distance
# 		position.z += cos(rotation.y) * distance
# 	if Input.is_action_pressed("LEFT"):
# 		position.x -= sin(rotation.y+(PI/2)) * distance
# 		position.z -= cos(rotation.y+(PI/2)) * distance
# 	if Input.is_action_pressed("RIGHT"):
# 		position.x += sin(rotation.y+(PI/2)) * distance
# 		position.z += cos(rotation.y+(PI/2)) * distance
