import engine

def constructVec3Test() -> bool:
    v1 = engine.Vector3(3, 4, 5)
    print(v1)
    a = v1.x
    return bool((v1.x == 3) and (v1.y == 4) and (v1.z == 5))

print(constructVec3Test())