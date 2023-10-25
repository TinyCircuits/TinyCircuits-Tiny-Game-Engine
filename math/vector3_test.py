import engine
import math

def float_compare_eq(a, b, tol=1E-7) -> bool:
    return (abs(a-b) <= tol)

def constructVec3Test() -> bool:
    v1 = engine.Vector3(3, 4, 5)
    return ((v1.x == 3) and (v1.y == 4) and (v1.z == 5))

def normalizeVec3Test() -> bool:
    v1 = engine.Vector3(3, 4, 5)
    v1.normalize()
    return (float_compare_eq(v1.x, 3 / math.sqrt(50)) and float_compare_eq(v1.y, 4 / math.sqrt(50)) and float_compare_eq(v1.z, 5 / math.sqrt(50)))

resultDict = {}

testDict = {
    "ConstructVec3": constructVec3Test,
    "NormalizeVec3": normalizeVec3Test,
}
        
def print_results():
    for testKey in testDict:
        print("TEST: "+ testKey)
        print("\tPASSED" if resultDict[testKey] else "\tFAILED")
        
def run_tests():
    for testKey in testDict:
        print("RUNNING TEST: "+testKey)
        resultDict[testKey] = testDict[testKey]()
    print_results()
    


run_tests()