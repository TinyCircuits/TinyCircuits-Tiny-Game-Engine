import engine
from engine_math import Vector2
import math
from common import run_tests

def float_compare_eq(a, b, tol=1E-7) -> bool:
    return (abs(a-b) <= tol)

def constructVec2Test() -> bool:
    v1 = engine.Vector2(5, 10)
    return ((v1.x == 5) and (v1.y == 10))

def normalizeVec2Test() -> bool:
    v1 = engine.Vector2(5, 10)
    v1.normalize()
    return (float_compare_eq(v1.x, 5 / math.sqrt(125)) and float_compare_eq(v1.y, 10 / math.sqrt(125)))

def rotateVec2Test() -> bool:
    v1 = engine.Vector2(1, 0)
    
    v1.rotateZ(math.pi)
    
    return (float_compare_eq(v1.x, -1) and float_compare_eq(v1.y, 0))

def resizeVec2VecTest() -> bool:
    v1 = engine.Vector2(5, 10)
    v2 = engine.Vector2(2, 3)
    
    v1.resize(v2)
    return float_compare_eq(v1.len2(), v2.len2())

def resizeVec2ScalarTest() -> bool:
    v1 = engine.Vector2(5, 10)
    mag = 5.0
    v1.resize(mag)
    return (float_compare_eq(v1.len2(), mag*mag) and float_compare_eq(v1.cross(engine.Vector2(5, 10)), 0))

        
test_list = [
    ["ConstructVec2", constructVec2Test],
    ["NormalizeVec2", normalizeVec2Test],
    ["RotateVec2", rotateVec2Test],
    ["ResizeVec2Vec", resizeVec2VecTest],
    ["ResizeVec2Scalar", resizeVec2ScalarTest],
]

run_tests(test_list)