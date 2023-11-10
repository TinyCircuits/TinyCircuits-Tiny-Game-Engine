import gc
import engine
from common import run_tests
from engine import Sprite2DNode, Vector2

engine.debug_enable_all()
engine.init()


def non_inherited_attribute_xy_pos_test():
    non_inherited = Sprite2DNode()
    non_inherited.position.x = 9
    non_inherited.position.y = 9
    return non_inherited.position.x == 9 and non_inherited.position.y == 9

def non_inherited_attribute_pos_test():
    non_inherited = Sprite2DNode()
    non_inherited.position = Vector2(10, 10)
    return non_inherited.position.x == 10 and non_inherited.position.y == 10

def non_inherited_attribute_width_test():
    non_inherited = Sprite2DNode()
    non_inherited.width = 11
    return non_inherited.width == 11

def non_inherited_attribute_height_test():
    non_inherited = Sprite2DNode()
    non_inherited.height = 12
    return non_inherited.height == 12

def non_inherited_attribute_filename_test():
    non_inherited = Sprite2DNode()
    non_inherited.filename = "filename.bmp"
    return non_inherited.filename == "filename.bmp"

def non_inherited_attribute_fps_test():
    non_inherited = Sprite2DNode()
    non_inherited.fps = 60
    return non_inherited.fps == 60

def non_inherited_attribute_rotation_test():
    non_inherited = Sprite2DNode()
    non_inherited.rotation = 75.5
    return non_inherited.rotation == 75.5

def non_inherited_attribute_xy_scale_test():
    non_inherited = Sprite2DNode()
    non_inherited.scale.x = 9
    non_inherited.scale.y = 9
    return non_inherited.scale.x == 9 and non_inherited.scale.y == 9

def non_inherited_attribute_scale_test():
    non_inherited = Sprite2DNode()
    non_inherited.scale = Vector2(10, 10)
    return non_inherited.scale.x == 10 and non_inherited.scale.y == 10

def non_inherited_set_layer_test():
    non_inherited = Sprite2DNode()
    non_inherited.set_layer(6)
    return non_inherited.get_layer() == 6


def inherited_attribute_init_xy_pos_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.position.x = 9
            self.position.y = 9

    inherited = MySprite2D()

    return inherited.position.x == 9 and inherited.position.y == 9

def inherited_attribute_init_pos_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.position = Vector2(10, 10)

    inherited = MySprite2D()

    return inherited.position.x == 10 and inherited.position.y == 10

def inherited_attribute_init_width_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.width = 11

    inherited = MySprite2D()

    return inherited.width == 11

def inherited_attribute_init_height_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.height = 12

    inherited = MySprite2D()

    return inherited.height == 12

def inherited_attribute_init_filename_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.filename = "filename.bmp"

    inherited = MySprite2D()

    return inherited.filename == "filename.bmp"

def inherited_attribute_init_fps_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.fps = 60

    inherited = MySprite2D()

    return inherited.fps == 60

def inherited_attribute_init_rotation_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.rotation = 75.5

    inherited = MySprite2D()

    return inherited.rotation == 75.5

def inherited_attribute_init_xy_scale_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.scale.x = 9
            self.scale.y = 9

    inherited = MySprite2D()

    return inherited.scale.x == 9 and inherited.scale.y == 9

def inherited_attribute_init_scale_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.scale = Vector2(10, 10)

    inherited = MySprite2D()

    return inherited.scale.x == 10 and inherited.scale.y == 10

def inherited_attribute_init_layer_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
            self.set_layer(7)

    inherited = MySprite2D()

    return inherited.get_layer() == 7




def inherited_attribute_tick_xy_pos_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.position.x = 9
            self.position.y = 9

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 9 and inherited.position.y == 9

def inherited_attribute_tick_pos_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.position = Vector2(10, 10)

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.position.x == 10 and inherited.position.y == 10

def inherited_attribute_tick_width_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.width = 11

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.width == 11

def inherited_attribute_tick_height_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.height = 12

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.height == 12

def inherited_attribute_tick_filename_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.filename = "filename.bmp"

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.filename == "filename.bmp"

def inherited_attribute_tick_fps_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.fps = 60

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.fps == 60

def inherited_attribute_tick_rotation_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.rotation = 75.5

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.rotation == 75.5

def inherited_attribute_tick_xy_scale_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.scale.x = 9
            self.scale.y = 9

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.scale.x == 9 and inherited.scale.y == 9

def inherited_attribute_tick_scale_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.scale = Vector2(10, 10)

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    return inherited.scale.x == 10 and inherited.scale.y == 10

def inherited_attribute_tick_layer_test():
    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)
        
        def tick(self):
            self.set_layer(7)

    inherited = MySprite2D()

    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    print(inherited.get_layer())
    return inherited.get_layer() == 7




def non_inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    # Get rid of any objects that maybe waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    non_inherited = Sprite2DNode()
    del non_inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before



def inherited_attribute_gc_collect_test():
    # Two ticks since object would be just added
    engine.tick()
    engine.tick()

    class MySprite2D(Sprite2DNode):
        def __init__(self):
            super().__init__(self)

    # Get rid of any objects that maybe waiting to be deleted too
    gc.collect()

    # Get the count before adding another one, add one, then delete and collect it
    count_before = engine.get_total_object_count()
    inherited = MySprite2D()
    del inherited
    gc.collect()

    # See if we're back to where we were before
    return engine.get_total_object_count() == count_before


test_list = [
    ["non_inherited_attribute_xy_pos_test", non_inherited_attribute_xy_pos_test],
    ["non_inherited_attribute_pos_test", non_inherited_attribute_pos_test],
    ["non_inherited_attribute_width_test", non_inherited_attribute_width_test],
    ["non_inherited_attribute_height_test", non_inherited_attribute_height_test],
    ["non_inherited_attribute_filename_test", non_inherited_attribute_filename_test],
    ["non_inherited_attribute_fps_test", non_inherited_attribute_fps_test],
    ["non_inherited_attribute_rotation_test", non_inherited_attribute_rotation_test],
    ["non_inherited_attribute_xy_scale_test", non_inherited_attribute_xy_scale_test],
    ["non_inherited_attribute_scale_test", non_inherited_attribute_scale_test],
    ["non_inherited_set_layer_test", non_inherited_set_layer_test],

    ["inherited_attribute_init_xy_pos_test", inherited_attribute_init_xy_pos_test],
    ["inherited_attribute_init_pos_test", inherited_attribute_init_pos_test],
    ["inherited_attribute_init_width_test", inherited_attribute_init_width_test],
    ["inherited_attribute_init_height_test", inherited_attribute_init_height_test],
    ["inherited_attribute_init_filename_test", inherited_attribute_init_filename_test],
    ["inherited_attribute_init_fps_test", inherited_attribute_init_fps_test],
    ["inherited_attribute_init_rotation_test", inherited_attribute_init_rotation_test],
    ["inherited_attribute_init_xy_scale_test", inherited_attribute_init_xy_scale_test],
    ["inherited_attribute_init_scale_test", inherited_attribute_init_scale_test],
    ["inherited_attribute_init_layer_test", inherited_attribute_init_layer_test],

    ["inherited_attribute_tick_xy_pos_test", inherited_attribute_tick_xy_pos_test],
    ["inherited_attribute_tick_pos_test", inherited_attribute_tick_pos_test],
    ["inherited_attribute_tick_width_test", inherited_attribute_tick_width_test],
    ["inherited_attribute_tick_height_test", inherited_attribute_tick_height_test],
    ["inherited_attribute_tick_filename_test", inherited_attribute_tick_filename_test],
    ["inherited_attribute_tick_fps_test", inherited_attribute_tick_fps_test],
    ["inherited_attribute_tick_rotation_test", inherited_attribute_tick_rotation_test],
    ["inherited_attribute_tick_xy_scale_test", inherited_attribute_tick_xy_scale_test],
    ["inherited_attribute_tick_scale_test", inherited_attribute_tick_scale_test],
    ["inherited_attribute_tick_layer_test", inherited_attribute_tick_layer_test],

    ["non_inherited_attribute_gc_collect_test", non_inherited_attribute_gc_collect_test],
    ["inherited_attribute_gc_collect_test", inherited_attribute_gc_collect_test]
]


run_tests(test_list)