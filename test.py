import engine
from engine import EmptyNode
import gc
print("dir(engine):",dir(engine))
print("dir(EmptyNode):", dir(EmptyNode))


engine.set_debug_print_level(engine.debug_print_level_all)


class MyNodeA(EmptyNode):
    def __init__(self):
        super().__init__(self)

    def tick(self):
        print("hi from tick() A")


class MyNodeB(EmptyNode):
    def __init__(self):
        super().__init__(self)


class MyNodeC(EmptyNode):
    def __init__(self):
        super().__init__(self)


class MyNodeD(EmptyNode):
    def __init__(self):
        super().__init__(self)
    
    def tick(self):
        # pass
        print("hi from tick() D")


a = MyNodeA()
b = MyNodeB()
if(True):
    c = MyNodeC()
    del c
d = MyNodeD()
d.set_layer(7)

gc.collect()

engine.start()






# print("# List of importable modules:")
# exec("help('modules')")
# print("")

# import simpleclass

# a = simpleclass.myclass(2, 3)
# print(a)
# print(a.mysum())

# b = simpleclass.myclass(3, 3)
# print(b)
# print(b.mysum())

# print(dir(simpleclass))
# print(b)



# print(a.set_callback)

# def test2():
#     print("hi 1")

# a.test = a.set_callback(test2)
# a.test()
# # a.test2()