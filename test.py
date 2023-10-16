# There are two ways the API could look.

# # Way #1:
# import engine

# my_new_node = engine.Node()

# def my_tick_callback():
#     print("Hi I am a node! I like to move around!")

# my_new_node.set_callback(my_tick_callback)

# engine.start()


# # Way #2
# import engine

# class MyNewNode(engine.Node):
#     def __init__(self):
#         # I could do extra setup stuff here!
#         pass

#     # The name has to be this!
#     def tick():
#         # I can do anything here like move, print, or whatever
#         pass

import engine
from engine import BaseNode
import gc
print("dir(engine):",dir(engine))
print("dir(BaseNode):", dir(BaseNode))


engine.set_debug_print_level(engine.debug_print_level_all)


class MyNodeA(BaseNode):
    def __init__(self):
        super().__init__(self)

    def tick(self):
        print("hi from tick()")


class MyNodeB(BaseNode):
    def __init__(self):
        super().__init__(self)


a = MyNodeA()
b = MyNodeB()
if(True):
    c = MyNodeA()
    del c

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