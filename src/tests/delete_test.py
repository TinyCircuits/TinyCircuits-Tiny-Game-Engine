import engine
from engine_nodes import Circle2DNode

import gc

# c0 = Circle2DNode()
# del c0

class MyCircle(Circle2DNode):
    def __init__(self):
        super().__init__(self)

# class A:
#     def __init__(self):
#         pass

# class B(A):
#     def __init__(self):
#         pass

c1 = MyCircle()
del c1

# b = B()
# del b

# print(c0, c1, b)

l = []

while True:
    l.append(0)
    gc.collect()