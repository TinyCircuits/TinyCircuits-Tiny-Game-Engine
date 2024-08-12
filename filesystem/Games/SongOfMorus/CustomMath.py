
from engine_math import Vector2,Vector3
import math

@micropython.native
def world_to_screen_point(wp,camX,camY,mat):

    res = Vector3(wp.x - camX,  wp.y- camY,    wp.z)

    #temp = [
    #    mat[0][0] * wp_world[0] + mat[0][1] * wp_world[1] + mat[0][2] * wp_world[2] + mat[0][3] * wp_world[3],
    #    mat[1][0] * wp_world[0] + mat[1][1] * wp_world[1] + mat[1][2] * wp_world[2] + mat[1][3] * wp_world[3],
    #    mat[2][0] * wp_world[0] + mat[2][1] * wp_world[1] + mat[2][2] * wp_world[2] + mat[2][3] * wp_world[3],
    #    mat[3][0] * wp_world[0] + mat[3][1] * wp_world[1] + mat[3][2] * wp_world[2] + mat[3][3] * wp_world[3]
    #]
    temp = [
        mat[0][0] * res.x ,
        mat[1][1] * res.y + mat[1][2] * res.z ,
        mat[2][2] * res.z + mat[2][3] ,
        mat[3][2] * res.z
    ]


    if temp[3] == 0:
       return Vector3(0,0,0)
    else:
        res.x = (temp[0] / temp[3] )  * 64
        res.y = (temp[1] / temp[3] )  * 64
        return res
    
@micropython.native
def screen_to_direction(screen_pos , camMatrix11, camMatrix12):
    screen_pos.x =    screen_pos.x / 128 - 0.5
    screen_pos.y =    screen_pos.y / 128 - 0.5

    world_height = 2.0 * (1.0 /camMatrix11)

    world_units = Vector3(screen_pos.x * world_height,  (screen_pos.y+camMatrix12/2) * world_height,1)
    len = world_units.x*world_units.x + world_units.y*world_units.y + world_units.z*world_units.z
    len = math.sqrt(len)
    world_units.x /= len
    world_units.y /= len
    world_units.z /= len
    return world_units

@micropython.native
def rotate2Axis(direction, angleX,angleY):

    direction.y =  math.cos(angleX) * direction.y - math.sin(angleX) * direction.z
    direction.z =  math.sin(angleX) * direction.y + math.cos(angleX) * direction.z
    direction.x =  math.cos(angleY) * direction.x - math.sin(angleY) * direction.z
    direction.z =  math.sin(angleY) * direction.x + math.cos(angleY) * direction.z
    return direction

    #return world_units.normalized()