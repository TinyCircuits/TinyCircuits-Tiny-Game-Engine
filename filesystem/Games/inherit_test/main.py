import engine_main
import engine
import engine_io

from engine_math import Vector2
from engine_resources import TextureResource, FontResource
from engine_nodes import Circle2DNode, Rectangle2DNode, CameraNode, GUIBitmapButton2DNode, GUIButton2DNode, Line2DNode, PhysicsCircle2DNode, PhysicsRectangle2DNode

import os
os.chdir("Games/inherit_test")

btn_bitmap = TextureResource("button.bmp")
spr_bitmap = TextureResource("32x32.bmp")
font = FontResource("9pt-roboto-font.bmp")


# Circle2DNode test
cir_parent = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 10), radius=10)
cir_child0 = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 10), radius=10)
cir_child1 = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 10), radius=10)

cir_parent.add_child(cir_child0)
cir_child0.add_child(cir_child1)


# GUIBitmapButton2DNode test
bitmap_btn_parent = GUIBitmapButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(120, 10), bitmap=btn_bitmap, font=font, text="1")
bitmap_btn_child0 = GUIBitmapButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 10), bitmap=btn_bitmap, font=font, text="2")
bitmap_btn_child1 = GUIBitmapButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 10), bitmap=btn_bitmap, font=font, text="3")

bitmap_btn_parent.add_child(bitmap_btn_child0)
bitmap_btn_child0.add_child(bitmap_btn_child1)


# GUIButton2DNode test
btn_parent = GUIButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(240, 10), font=font, text="1")
btn_child0 = GUIButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 10), font=font, text="2")
btn_child1 = GUIButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 10), font=font, text="3")

btn_parent.add_child(btn_child0)
btn_child0.add_child(btn_child1)


# Line2DNode test
line_parent = Line2DNode(opacity=0.75, start=Vector2(-140, 10), end=Vector2(-100, 10))
line_cir_child0 = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 10), radius=10)
line_child1 = Line2DNode(opacity=0.75, start=Vector2(-20, 10), end=Vector2(20, 10))
line_child2 = Line2DNode(opacity=0.75, start=Vector2(-20, -10), end=Vector2(20, 10))

line_parent.add_child(line_cir_child0)
line_cir_child0.add_child(line_child1)
line_child1.add_child(line_child2)


# PhysicsCircle2DNode test
phy_cir_parent = PhysicsCircle2DNode(position=Vector2(0, -200), radius=10, outline=True, dynamic=False)
phy_cir_child0 = PhysicsCircle2DNode(position=Vector2(0, 10), radius=10, outline=True, dynamic=False)
phy_cir_child1 = PhysicsCircle2DNode(position=Vector2(0, 10), radius=10, outline=True, dynamic=False)

phy_cir_parent.add_child(phy_cir_child0)
phy_cir_child0.add_child(phy_cir_child1)


# PhysicsRectangle2DNode test
phy_rect_parent = PhysicsRectangle2DNode(position=Vector2(0, 200), outline=True, dynamic=False)
phy_rect_child0 = PhysicsRectangle2DNode(position=Vector2(0, 10), outline=True, dynamic=False)
phy_rect_child1 = PhysicsRectangle2DNode(position=Vector2(0, 10), outline=True, dynamic=False)

phy_rect_parent.add_child(phy_rect_child0)
phy_rect_child0.add_child(phy_rect_child1)


# Rectangle2DNode test
rect_parent = Rectangle2DNode(position=Vector2(440, 10), scale=Vector2(2, 2), opacity=0.75)
rect_child0 = Rectangle2DNode(position=Vector2(0, 10), scale=Vector2(2, 2), opacity=0.75)
rect_child1 = Rectangle2DNode(position=Vector2(0, 10), scale=Vector2(2, 2), opacity=0.75)

rect_parent.add_child(rect_child0)
rect_child0.add_child(rect_child1)


class TestCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.zoom = 0.6
    
    def tick(self, dt):
        if(engine_io.UP.is_pressed):
            self.position.y -= 1
        elif(engine_io.DOWN.is_pressed):
            self.position.y += 1

        if(engine_io.RIGHT.is_pressed):
            self.position.x += 1
        elif(engine_io.LEFT.is_pressed):
            self.position.x -= 1
        
        if(engine_io.A.is_pressed):
            self.zoom += 0.001
        elif(engine_io.B.is_pressed):
            self.zoom -= 0.001
        
        if(engine_io.LB.is_pressed):
            self.rotation.z += 0.01
        elif(engine_io.RB.is_pressed):
            self.rotation.z -= 0.01
        

cam = TestCamera()
cam.opacity = 1.0

while True:
    if engine.tick():
        cir_parent.rotation += 0.01