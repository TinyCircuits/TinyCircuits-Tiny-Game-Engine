import engine
from engine_math import Vector3
from common import run_tests
import math

def float_compare_eq(a, b, tol=1E-7) -> bool:
    return (abs(a-b) <= tol)

def constructVec3Test() -> bool:
    v1 = Vector3(3, 4, 5)
    return ((v1.x == 3) and (v1.y == 4) and (v1.z == 5))

def normalizeVec3Test() -> bool:
    v1 = Vector3(3, 4, 5)
    v1.normalize()
    return (float_compare_eq(v1.x, 3 / math.sqrt(50)) and float_compare_eq(v1.y, 4 / math.sqrt(50)) and float_compare_eq(v1.z, 5 / math.sqrt(50)))

def rotateVec3Test() -> bool:
    v1 = Vector3(0, 1, 0)
    axis = Vector3(math.sqrt(2)/2, math.sqrt(2)/2, 0)
    
    v1.rotate(axis, math.pi)
    
    return (float_compare_eq(v1.x, 1) and float_compare_eq(v1.y, 0) and float_compare_eq(v1.z, 0))

def resizeVec3VecTest() -> bool:
    v1 = Vector3(3, 4, 5)
    v2 = Vector3(3, 4, 0)
    
    v1.resize(v2)
    return (float_compare_eq(v1.len_squared(), v2.len_squared()) and float_compare_eq(v1.cross(Vector3(3, 4, 5)).len_squared(), 0))

def resizeVec3ScalarTest() -> bool:
    v1 = Vector3(3, 4, 5)
    mag = 5.0
    v1.resize(mag)
    print(v1.cross(Vector3(3, 4, 5)))
    return (float_compare_eq(v1.len_squared(), mag*mag) and float_compare_eq(v1.cross(Vector3(3, 4, 5)).len_squared(), 0))



test_list = [
    ["ConstructVec3", constructVec3Test],
    ["NormalizeVec3", normalizeVec3Test],
    ["RotateVec3", rotateVec3Test],
    ["ResizeVec3Vec", resizeVec3VecTest],
    ["ResizeVec3Scalar", resizeVec3ScalarTest],
]

run_tests(test_list)