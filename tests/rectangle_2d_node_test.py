import gc
import engine
from common import run_tests
from engine import Rectangle2DNode, Vector2

engine.debug_enable_all()
engine.init()

def non_inherited_attribute_xy_pos_test():
    non_inherited = Rectangle2DNode()
    non_inherited.position.x = 9
    non_inherited.position.y = 9
    return non_inherited.position.x == 9 and non_inherited.position.y == 9

def non_inherited_attribute_pos_test():
    non_inherited = Rectangle2DNode()
    non_inherited.position = Vector2(10, 10)
    return non_inherited.position.x == 10 and non_inherited.position.y == 10

def non_inherited_attribute_width_test():
    non_inherited = Rectangle2DNode()
    non_inherited.width = 11
    return non_inherited.width == 11

def non_inherited_attribute_height_test():
    non_inherited = Rectangle2DNode()
    non_inherited.height = 12
    return non_inherited.height == 12

def non_inherited_attribute_color_test():
    non_inherited = Rectangle2DNode()
    non_inherited.color = 13
    return non_inherited.color == 13

def non_inherited_set_layer_test():
    non_inherited = Rectangle2DNode()
    non_inherited.set_layer(6)
    return non_inherited.get_layer() == 6




def inherited_attribute_init_xy_pos_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.position.x = 9
            self.position.y = 9

    inherited = MyRect2D()

    return inherited.position.x == 9 and inherited.position.y == 9

def inherited_attribute_init_pos_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.position = Vector2(10, 10)

    inherited = MyRect2D()

    return inherited.position.x == 10 and inherited.position.y == 10

def inherited_attribute_init_width_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.width = 11

    inherited = MyRect2D()

    return inherited.width == 11

def inherited_attribute_init_height_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.height = 12

    inherited = MyRect2D()

    return inherited.height == 12

def inherited_attribute_init_color_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.color = 13

    inherited = MyRect2D()

    return inherited.color == 13

def inherited_attribute_init_layer_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
            self.set_layer(7)

    inherited = MyRect2D()

    return inherited.get_layer() == 7




def inherited_attribute_tick_xy_pos_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.position.x = 9
            self.position.y = 9

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 9 and inherited.position.y == 9

def inherited_attribute_tick_pos_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.position = Vector2(10, 10)

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 10 and inherited.position.y == 10

def inherited_attribute_tick_width_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.width = 11

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.width == 11

def inherited_attribute_tick_height_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.height = 12

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.height == 12

def inherited_attribute_tick_color_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.color = 13

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.color == 13

def inherited_attribute_tick_layer_test():
    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.set_layer(7)

    inherited = MyRect2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.get_layer() == 7




def non_inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    # Get rid of any objects that maybe waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    non_inherited = Rectangle2DNode()
    del non_inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before



def inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    class MyRect2D(Rectangle2DNode):
        def __init__(self):
            super().__init__(self)

    # Get rid of any objects that maybe waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    inherited = MyRect2D()
    del inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before


test_list = [
    ["non_inherited_attribute_xy_pos_test", non_inherited_attribute_xy_pos_test],
    ["non_inherited_attribute_pos_test", non_inherited_attribute_pos_test],
    ["non_inherited_attribute_width_test", non_inherited_attribute_width_test],
    ["non_inherited_attribute_height_test", non_inherited_attribute_height_test],
    ["non_inherited_attribute_color_test", non_inherited_attribute_color_test],
    ["non_inherited_set_layer_test", non_inherited_set_layer_test],

    ["inherited_attribute_init_xy_pos_test", inherited_attribute_init_xy_pos_test],
    ["inherited_attribute_init_pos_test", inherited_attribute_init_pos_test],
    ["inherited_attribute_init_width_test", inherited_attribute_init_width_test],
    ["inherited_attribute_init_height_test", inherited_attribute_init_height_test],
    ["inherited_attribute_init_color_test", inherited_attribute_init_color_test],
    ["inherited_attribute_init_layer_test", inherited_attribute_init_layer_test],

    ["inherited_attribute_tick_xy_pos_test", inherited_attribute_tick_xy_pos_test],
    ["inherited_attribute_tick_pos_test", inherited_attribute_tick_pos_test],
    ["inherited_attribute_tick_width_test", inherited_attribute_tick_width_test],
    ["inherited_attribute_tick_height_test", inherited_attribute_tick_height_test],
    ["inherited_attribute_tick_color_test", inherited_attribute_tick_color_test],
    ["inherited_attribute_tick_layer_test", inherited_attribute_tick_layer_test],

    ["non_inherited_attribute_gc_collect_test", non_inherited_attribute_gc_collect_test],
    ["inherited_attribute_gc_collect_test", inherited_attribute_gc_collect_test]
]


run_tests(test_list)