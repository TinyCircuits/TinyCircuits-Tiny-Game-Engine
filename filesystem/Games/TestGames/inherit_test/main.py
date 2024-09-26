import engine_main
import engine
import engine_io
import engine_draw

from engine_math import Vector2
from engine_resources import TextureResource, FontResource
from engine_nodes import Circle2DNode, Rectangle2DNode, Sprite2DNode, CameraNode, GUIBitmapButton2DNode, GUIButton2DNode, Line2DNode, PhysicsCircle2DNode, PhysicsRectangle2DNode, Text2DNode

import os
os.chdir("Games/TestGames/inherit_test")

btn_bitmap = TextureResource("button.bmp")
spr_bitmap = TextureResource("16x16.bmp")


engine.fps_limit(60)


# Outline Circle2DNode test
out_cir_label   = Text2DNode(text="Out Circle Test", position=Vector2(-20, -90))
out_cir_parent  = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 5-70), radius=5, outline=True)
out_cir_child   = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 5), radius=5, outline=True)
out_cir_compare = Circle2DNode(scale=2*2, opacity=0.75*0.75, position=Vector2(-5*2*2*2, (5*2*1.5)-70), radius=5, outline=True) # Should be the same size and opacity as the child
out_cir_parent.add_child(out_cir_child)


# Circle2DNode test
cir_label   = Text2DNode(text="Circle Test", position=Vector2(-20, -20))
cir_parent  = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 5), radius=5)
cir_child   = Circle2DNode(scale=2, opacity=0.75, position=Vector2(0, 5), radius=5)
cir_compare = Circle2DNode(scale=2*2, opacity=0.75*0.75, position=Vector2(-5*2*2*2, 5*2*1.5), radius=5) # Should be the same size and opacity as the child
cir_parent.add_child(cir_child)


# Outline Rectangle2DNode test
out_rect_label   = Text2DNode(text="Out Rectangle Test", position=Vector2(70, -90))
out_rect_parent  = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(91, 5-70), width=10, height=10, outline=True)
out_rect_child   = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 5), width=10, height=10, outline=True)
out_rect_compare = Rectangle2DNode(scale=Vector2(2*2, 2*2), opacity=0.75*0.75, position=Vector2(49, 15-70), width=10, height=10, outline=True)
out_rect_parent.add_child(out_rect_child)


# Rectangle2DNode test
rect_label   = Text2DNode(text="Rectangle Test", position=Vector2(70, -20))
rect_parent  = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(91, 5), width=10, height=10)
rect_child   = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 5), width=10, height=10)
rect_compare = Rectangle2DNode(scale=Vector2(2*2, 2*2), opacity=0.75*0.75, position=Vector2(49, 15), width=10, height=10)
rect_parent.add_child(rect_child)


# Sprite2DNode test
spr_label   = Text2DNode(text="Sprite Test", position=Vector2(-20, 60))
spr_parent  = Sprite2DNode(texture=spr_bitmap, scale=Vector2(2, 2), opacity=0.75, position=Vector2(-20+33, 90), transparent_color=engine_draw.black)
spr_child   = Sprite2DNode(texture=spr_bitmap, scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 8), transparent_color=engine_draw.black)
spr_compare = Sprite2DNode(texture=spr_bitmap, scale=Vector2(2*2, 2*2), opacity=0.75*0.75, position=Vector2(-20-33, 106), transparent_color=engine_draw.black)
spr_parent.add_child(spr_child)


# Text2DNode test (text height is 5)
text_label  = Text2DNode(text="Text Test", position=Vector2(180, 60))
text_parent = Text2DNode(scale=Vector2(2, 2), text="Hello!", position=Vector2(180+60, 80))
text_child  = Text2DNode(scale=Vector2(2, 2), text="Hello!", position=Vector2(0, 2.5))
text_label  = Text2DNode(scale=Vector2(2*2, 2*2), text="Hello!", position=Vector2(180-60, 85))
text_parent.add_child(text_child)


# Line2DNode test
line_label = Text2DNode(text="Line Test", position=Vector2(-20, 160))
line_parent = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(-20, 190), width=15, height=4)
line_child = Line2DNode(opacity=0.75, start=Vector2(-12.5, 0), end=Vector2(12.5, 0), thickness=4)
line_compare = Line2DNode(opacity=0.75*0.75, start=Vector2((-12.5*2)-80, 190), end=Vector2((12.5*2)-80, 190), thickness=4*2)    # Multiply end points and thickness by 2 to replicate scaling for comparison
line_parent.add_child(line_child)


# GUIBitmapButton2DNode test (height=14px)
gui_bitmap_label    = Text2DNode(text="GUI Bitmap test", position=Vector2(-200, -20))
gui_bitmap_parent   = GUIBitmapButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(-200+70, 10), bitmap=btn_bitmap, text="1")
gui_bitmap_child    = GUIBitmapButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, 7), bitmap=btn_bitmap, text="1")
gui_bitmap_compare  = GUIBitmapButton2DNode(scale=Vector2(2*2, 2*2), opacity=0.75*0.75, position=Vector2(-200-70, 14+9), bitmap=btn_bitmap, text="1")
gui_bitmap_parent.add_child(gui_bitmap_child)


# GUIButton2DNode test
gui_label   = Text2DNode(text="GUI test", position=Vector2(-200, 90))
gui_parent  = GUIButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(-200+40, 130), text="1")
gui_child   = GUIButton2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(0, gui_parent.height/2), text="1")
gui_compare = GUIButton2DNode(scale=Vector2(2*2, 2*2), opacity=0.75*0.75, position=Vector2(-200-40, 130+(gui_parent.height)), text="1")
gui_parent.add_child(gui_child)


# PhysicsCircle2DNode test
physics_label = Text2DNode(text="Physics test", position=Vector2(-200, 200))
physics_ground = PhysicsRectangle2DNode(dynamic=False, position=Vector2(-200, 325), outline=True, width=150, bounciness=0)

physics_circ_parent = Circle2DNode(scale=2, opacity=0.75, position=Vector2(-200-30, 220), radius=5)   # Radius will be 10 and everything under it should also scale by 2
physics_circ_child = PhysicsCircle2DNode(position=Vector2(0, 0), outline=True, bounciness=0, radius=5)  # Has the same radius as the parent, so will end up as the same size after scaling
physics_circ_parent.add_child(physics_circ_child)

physics_rect_parent = Rectangle2DNode(scale=Vector2(2, 2), opacity=0.75, position=Vector2(-200+30, 220))
physics_rect_child = PhysicsRectangle2DNode(position=Vector2(0, 0), outline=True, bounciness=0, rotation=0.47)
physics_rect_parent.add_child(physics_rect_child)

# # PhysicsCircle2DNode test
# phy_cir_parent = PhysicsCircle2DNode(position=Vector2(0, -200), radius=10, outline=True, dynamic=False)
# phy_cir_child0 = PhysicsCircle2DNode(position=Vector2(0, 10), radius=10, outline=True, dynamic=False)
# phy_cir_child1 = PhysicsCircle2DNode(position=Vector2(0, 10), radius=10, outline=True, dynamic=False)

# phy_cir_parent.add_child(phy_cir_child0)
# phy_cir_child0.add_child(phy_cir_child1)


# # PhysicsRectangle2DNode test
# phy_rect_parent = PhysicsRectangle2DNode(position=Vector2(0, 200), outline=True, dynamic=False)
# phy_rect_child0 = PhysicsRectangle2DNode(position=Vector2(0, 10), outline=True, dynamic=False)
# phy_rect_child1 = PhysicsRectangle2DNode(position=Vector2(0, 10), outline=True, dynamic=False)

# phy_rect_parent.add_child(phy_rect_child0)
# phy_rect_child0.add_child(phy_rect_child1)


class TestCamera(CameraNode):
    def __init__(self):
        super().__init__(self)
        self.zoom = 1.0
    
    def tick(self, dt):
        if(engine_io.UP.is_pressed):
            self.position.y -= 3
        elif(engine_io.DOWN.is_pressed):
            self.position.y += 3

        if(engine_io.RIGHT.is_pressed):
            self.position.x += 3
        elif(engine_io.LEFT.is_pressed):
            self.position.x -= 3
        
        if(engine_io.A.is_pressed):
            self.zoom += 0.01
        elif(engine_io.B.is_pressed):
            self.zoom -= 0.01
        
        if(engine_io.LB.is_pressed):
            self.rotation.z += 0.01
        elif(engine_io.RB.is_pressed):
            self.rotation.z -= 0.01
        

cam = TestCamera()
cam.opacity = 1.0
cross_hair = Circle2DNode(color=engine_draw.green, radius=2)
cam.add_child(cross_hair)

while True:
    if engine.tick():
        pass