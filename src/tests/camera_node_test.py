import gc
import engine
import engine_debug
from engine_nodes import CameraNode
from engine_math import Vector3, Rectangle
from common import run_tests

engine_debug.debug_enable_all()
engine.init()


def non_inherited_attribute_xyz_pos_test():
    non_inherited = CameraNode()
    non_inherited.position.x = 1
    non_inherited.position.y = 1
    non_inherited.position.z = 1
    return non_inherited.position.x == 1 and non_inherited.position.y == 1 and non_inherited.position.z == 1

def non_inherited_attribute_pos_test():
    non_inherited = CameraNode()
    non_inherited.position = Vector3(2, 2, 2)
    return non_inherited.position.x == 2 and non_inherited.position.y == 2 and non_inherited.position.z == 2

def non_inherited_attribute_xyz_rot_test():
    non_inherited = CameraNode()
    non_inherited.rotation.x = 1
    non_inherited.rotation.y = 1
    non_inherited.rotation.z = 1
    return non_inherited.rotation.x == 1 and non_inherited.rotation.y == 1 and non_inherited.rotation.z == 1

def non_inherited_attribute_rot_test():
    non_inherited = CameraNode()
    non_inherited.rotation = Vector3(2, 2, 2)
    return non_inherited.rotation.x == 2 and non_inherited.rotation.y == 2 and non_inherited.rotation.z == 2

def non_inherited_attribute_xywh_viewport_test():
    non_inherited = CameraNode()
    non_inherited.viewport.x = 3
    non_inherited.viewport.y = 3
    non_inherited.viewport.width = 3
    non_inherited.viewport.height = 3
    return non_inherited.viewport.x == 3 and non_inherited.viewport.y == 3 and non_inherited.viewport.width == 3 and non_inherited.viewport.height == 3

def non_inherited_attribute_viewport_test():
    non_inherited = CameraNode()
    non_inherited.viewport = Rectangle(4, 4, 4, 4)
    return non_inherited.viewport.x == 4 and non_inherited.viewport.y == 4 and non_inherited.viewport.width == 4 and non_inherited.viewport.height == 4


def inherited_attribute_init_xyz_pos_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.position.x = 1
            self.position.y = 1
            self.position.z = 1

    inherited = MyCam()
    return inherited.position.x == 1 and inherited.position.y == 1 and inherited.position.z == 1

def inherited_attribute_init_pos_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.position = Vector3(2, 2, 2)

    inherited = MyCam()
    return inherited.position.x == 2 and inherited.position.y == 2 and inherited.position.z == 2

def inherited_attribute_init_xyz_rot_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.rotation.x = 3
            self.rotation.y = 3
            self.rotation.z = 3

    inherited = MyCam()
    return inherited.rotation.x == 3 and inherited.rotation.y == 3 and inherited.rotation.z == 3

def inherited_attribute_init_rot_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.rotation = Vector3(4, 4, 4)

    inherited = MyCam()
    return inherited.rotation.x == 4 and inherited.rotation.y == 4 and inherited.rotation.z == 4

def inherited_attribute_init_xywh_viewport_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.viewport.x = 5
            self.viewport.y = 5
            self.viewport.width = 5
            self.viewport.height = 5

    inherited = MyCam()
    return inherited.viewport.x == 5 and inherited.viewport.y == 5 and inherited.viewport.width == 5 and inherited.viewport.height == 5

def inherited_attribute_init_viewport_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            self.viewport = Rectangle(6, 6, 6, 6)

    inherited = MyCam()
    return inherited.viewport.x == 6 and inherited.viewport.y == 6 and inherited.viewport.width == 6 and inherited.viewport.height == 6


def inherited_attribute_tick_xyz_pos_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.position.x = 1
            self.position.y = 1
            self.position.z = 1

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 1 and inherited.position.y == 1 and inherited.position.z == 1

def inherited_attribute_tick_pos_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            
        def tick(self):
            self.position = Vector3(2, 2, 2)

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 2 and inherited.position.y == 2 and inherited.position.z == 2

def inherited_attribute_tick_xyz_rot_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.rotation.x = 3
            self.rotation.y = 3
            self.rotation.z = 3

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.rotation.x == 3 and inherited.rotation.y == 3 and inherited.rotation.z == 3

def inherited_attribute_tick_rot_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.rotation = Vector3(4, 4, 4)

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.rotation.x == 4 and inherited.rotation.y == 4 and inherited.rotation.z == 4

def inherited_attribute_tick_xywh_viewport_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.viewport.x = 5
            self.viewport.y = 5
            self.viewport.width = 5
            self.viewport.height = 5

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.viewport.x == 5 and inherited.viewport.y == 5 and inherited.viewport.width == 5 and inherited.viewport.height == 5

def inherited_attribute_tick_viewport_test():
    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)
            
        def tick(self):
            self.viewport = Rectangle(6, 6, 6, 6)

    inherited = MyCam()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.viewport.x == 6 and inherited.viewport.y == 6 and inherited.viewport.width == 6 and inherited.viewport.height == 6



def non_inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    # Get rid of any objects that may be waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    non_inherited = CameraNode()
    del non_inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before



def inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    class MyCam(CameraNode):
        def __init__(self):
            super().__init__(self)

    # Get rid of any objects that may be waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    inherited = MyCam()
    del inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before



test_list = [
    ["non_inherited_attribute_xyz_pos_test", non_inherited_attribute_xyz_pos_test],
    ["non_inherited_attribute_pos_test", non_inherited_attribute_pos_test],
    ["non_inherited_attribute_xyz_rot_test", non_inherited_attribute_xyz_rot_test],
    ["non_inherited_attribute_rot_test", non_inherited_attribute_rot_test],
    ["non_inherited_attribute_xywh_viewport_test", non_inherited_attribute_xywh_viewport_test],
    ["non_inherited_attribute_viewport_test", non_inherited_attribute_viewport_test],

    ["inherited_attribute_init_xyz_pos_test", inherited_attribute_init_xyz_pos_test],
    ["inherited_attribute_init_pos_test", inherited_attribute_init_pos_test],
    ["inherited_attribute_init_xyz_rot_test", inherited_attribute_init_xyz_rot_test],
    ["inherited_attribute_init_rot_test", inherited_attribute_init_rot_test],
    ["inherited_attribute_init_xywh_viewport_test", inherited_attribute_init_xywh_viewport_test],
    ["inherited_attribute_init_viewport_test", inherited_attribute_init_viewport_test],

    ["inherited_attribute_tick_xyz_pos_test", inherited_attribute_tick_xyz_pos_test],
    ["inherited_attribute_tick_pos_test", inherited_attribute_tick_pos_test],
    ["inherited_attribute_tick_xyz_rot_test", inherited_attribute_tick_xyz_rot_test],
    ["inherited_attribute_tick_rot_test", inherited_attribute_tick_rot_test],
    ["inherited_attribute_tick_xywh_viewport_test", inherited_attribute_tick_xywh_viewport_test],
    ["inherited_attribute_tick_viewport_test", inherited_attribute_tick_viewport_test],

    ["non_inherited_attribute_gc_collect_test", non_inherited_attribute_gc_collect_test],
    ["inherited_attribute_gc_collect_test", inherited_attribute_gc_collect_test]
]


run_tests(test_list)