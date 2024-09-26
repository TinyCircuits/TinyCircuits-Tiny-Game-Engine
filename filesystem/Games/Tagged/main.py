engine.freq(250000000)

import os
# os.chdir("/Games/Tagged")

import math
import urandom
import utime
import gc

#gc.threshold(8*1024)

import engine_main

import random
import engine
import engine_io
import engine_physics
import engine_draw

from engine_draw import Color
from engine_nodes import Rectangle2DNode, CameraNode, Line2DNode, Circle2DNode, Sprite2DNode, PhysicsRectangle2DNode, PhysicsCircle2DNode, EmptyNode, Text2DNode
from engine_resources import TextureResource, FontResource, NoiseResource
from engine_math import Vector2, Vector3
from engine_animation import Tween, Delay, ONE_SHOT, LOOP, PING_PONG, EASE_ELAST_OUT, EASE_ELAST_IN_OUT, EASE_BOUNCE_IN_OUT, EASE_SINE_IN, EASE_QUAD_IN

#engine_debug.enable_all()

#engine.freq(__freq)

engine.fps_limit(50)

#engine.disable_fps_limit()

#runner_texture = TextureResource("test_runner_frames_2_16bit.bmp")


# S-room
s_room_flag_positions = (Vector2(0,48), Vector2(-52,-24), Vector2(52,-24))
s_room_p1_spawn = Vector2(0, 16)
s_room_p2_spawn = Vector2(0, -48)
s_room_color_pair = (engine_draw.darkgreen, engine_draw.green)
s_room_box_data = (
    (-64, -64, 2*math.sqrt(2)*24, math.sqrt(2)*24, math.pi/4),
    (64, -64, 2*math.sqrt(2)*24, math.sqrt(2)*24, -math.pi/4),
    (64, 64, 2*math.sqrt(2)*24, math.sqrt(2)*24, math.pi/4),
    (-64, 64, 2*math.sqrt(2)*24, math.sqrt(2)*24, -math.pi/4),
    (0, 64+8, 128, 16, 0),
    (0, -64-8, 128, 16, 0),
    (64+8, 0, 16, 128, 0),
    (-64-8, 0, 16, 128, 0),
    (-24, 40, 16, 16, 0),
    (24, -40, 16, 16, 0),

    (24, 24, 3*math.sqrt(2)*8, math.sqrt(2)*8, -math.pi/4),
    (-24, -24, 3*math.sqrt(2)*8, math.sqrt(2)*8, -math.pi/4),
    (0, 28, 64, 8, 0),
    (0, -28, 64, 8, 0),
    (12, 12, 8, 40, 0),
    (-12, -12, 8, 40, 0),

    (64, 0, math.sqrt(2)*16, math.sqrt(2)*16, math.pi/4),
    (-64, 0, math.sqrt(2)*16, math.sqrt(2)*16, -math.pi/4),
)


# Y-room
y_room_flag_positions = (Vector2(56, 40), Vector2(-56, 40), Vector2(0, 16))
y_room_p1_spawn = Vector2(-56, -32)
y_room_p2_spawn = Vector2(56, -32)
y_room_color_pair = (engine_draw.purple, engine_draw.violet)
y_room_box_data = (
    # Bottom corner squares

    (56, 56, 16, 16, 0),#engine_draw.set_background_color(engine_draw.darkgreen)
    (-56, 56, 16, 16, 0),

    # Side squares

    (56, 0, 16, 16, 0),
    (-56, 0, 16, 16, 0),

    (20, -12, math.sqrt(2)*8, math.sqrt(2)*8, math.pi/4),

    (-20, -12, math.sqrt(2)*8, math.sqrt(2)*8, -math.pi/4),

    # Walljump corridor

    (16, 12, 8, 48, 0),
    (-16, 12, 8, 48, 0),

    # Top, bottom, and wallsroom_p1_spawn = Vector2(-56, -32)
    (0, 64+8, 192, 16, 0),
    (0, -64-8, 192, 16, 0),
    (64+8, 0, 16, 192, 0),
    (-64-8, 0, 16, 192, 0),

    # Top-center platform
    (0, -44, 32, 8, 0)

)


# Springboard room
springboard_room_flag_positions = (Vector2(-48,48), Vector2(48,-48),)
springboard_room_p1_spawn = Vector2(0, -16)
springboard_room_p2_spawn = Vector2(0, 48)
springboard_room_color_pair = (engine_draw.darkcyan, engine_draw.cyan)
springboard_room_box_data = (
    # Top, bottom, and walls
    (0, 64+8, 192, 16, 0),
    (0, -64-8, 192, 16, 0),
    (64+8, 0, 16, 192, 0),
    (-64-8, 0, 16, 192, 0),

    # Center platform
    (0, 0, math.sqrt(2)*8, math.sqrt(2)*8, math.pi/4),
    (-8, 4, 16, 8, 0),
    (8, -4, 16, 8, 0),

    # Parkour platforms
    (36, 20, 8, 8, 0),
    (-36, -20, 8, 8, 0),

    # Ramps
    (-32, 52, math.sqrt(2)*32, math.sqrt(2)*8, math.pi/4),
    (32, -52, math.sqrt(2)*32, math.sqrt(2)*8, math.pi/4),

    (-16, 48, 8, 32, 0),
    (16, -48, 8, 32, 0),

    #(-28, 56, 16, 16, 0),
    #(28, -56, 16, 16, 0),60
)


thebox_room_flag_positions = (Vector2(-48,-24), Vector2(52,-52), Vector2(0,32), )
# The Box
thebox_room_p1_spawn = Vector2(0, -56)
thebox_room_p2_spawn = Vector2(0, 0)
thebox_room_color_pair = (engine_draw.darkgrey, engine_draw.lightgrey)
thebox_room_box_data = (
    # Top, bottom, and walls
    (0, 64+8, 192, 16, 0),
    (0, -64-8, 192, 16, 0),
    (64+8, 0, 16, 192, 0),
    (-64-8, 0, 16, 192, 0),

    # Inner box
    (-24, 20, 32, 8, 0),
    (28, 20, 40, 8, 0),
    (-28, 0, 8, 48, 0),
    (28, 0, 8, 48, 0),
    (0, -20, 64, 8, 0),

    # Left chamber
    (-44, 56, 8, 16, 0),
    (-60, 28, 8, 8, 0),
    (-56, -4, 16, 8, 0),
    (-44, -52, 8, 24, 0),

    # Right chamber
    (60, 44, 8, 8, 0),
    (60, -12, 8, 8, 0),
    (60, -36, 8, 8, 0),

    # Top chamber
    # (4, -44, 56, 8, 0),

    # Bottom chamber
    (-16, 44, 16, 8, 0),
    (24, 44, 16, 8, 0),

)


class LevelSelector(Rectangle2DNode):
    def __init__(self):
        super().__init__(self)
        self.width = 129
        self.height = 12
        self.outline = True
        self.layer = 7
        self.tween = Tween()
        self.selection = 0

    def tick(self, dt):
        if(engine_io.DOWN.is_just_pressed and self.selection < 3):
            self.selection += 1
            self.tween.start(self, "position", self.position, Vector2(0, 16 * self.selection), 150, 1.0, ONE_SHOT, EASE_QUAD_IN)
        if(engine_io.UP.is_just_pressed and self.selection > 0):
            self.selection -= 1
            self.tween.start(self, "position", self.position, Vector2(0, 16 * self.selection), 150, 1.0, ONE_SHOT, EASE_QUAD_IN)
        if(self.selection == 0):
            engine_draw.set_background_color(s_room_color_pair[0])
        elif(self.selection == 1):
            engine_draw.set_background_color(y_room_color_pair[0])
        elif(self.selection == 2):
            engine_draw.set_background_color(springboard_room_color_pair[0])
        elif(self.selection == 3):
            engine_draw.set_background_color(thebox_room_color_pair[0])
        pass

font = FontResource("outrunner_outline.bmp")
font2 = FontResource("9pt-roboto-font.bmp")

cam = CameraNode()
cam.position = Vector3(0, 0, 1)
engine_physics.set_gravity(0.0, -0.1)

selector = LevelSelector()

splash_text = Text2DNode()
splash_text.layer = 7
splash_text.font = font
splash_text.text = "TAGGED"
splash_text.position = Vector2(0, -32)

s_room_text = Text2DNode()
s_room_text.layer = 7
s_room_text.font = font
s_room_text.text = "S-room"
s_room_text.position = Vector2(0, 0)

y_room_text = Text2DNode()
y_room_text.layer = 7
y_room_text.font = font
y_room_text.text = "Y-room"
y_room_text.position = Vector2(0, 16)

springboards_room_text = Text2DNode()
springboards_room_text.layer = 7
springboards_room_text.font = font
springboards_room_text.text = "Springboards"
springboards_room_text.position = Vector2(0, 32)

thebox_room_text = Text2DNode()
thebox_room_text.layer = 7
thebox_room_text.font = font
thebox_room_text.text = "The Box"
thebox_room_text.position = Vector2(0, 48)

two_player_text = Text2DNode()
two_player_text.layer = 7
two_player_text.font = font
two_player_text.text = "2PLAYER >>"
two_player_text.position = Vector2(24, -58)

text_crossout = Rectangle2DNode()
text_crossout.layer = 7
text_crossout.outline = True
text_crossout.width = 64
text_crossout.height = 1
text_crossout.position = Vector2(24, -58)

under_construction_text = Text2DNode()
under_construction_text.layer = 7
under_construction_text.font = font2
under_construction_text.text = "Under construction"
under_construction_text.position = Vector2(20, -50)


cam.add_child(splash_text)
cam.add_child(s_room_text)
cam.add_child(y_room_text)
cam.add_child(springboards_room_text)
cam.add_child(thebox_room_text)
cam.add_child(two_player_text)
cam.add_child(under_construction_text)
cam.add_child(selector)

flag_positions = None
room_p1_spawn = Vector2(-32, 0)
room_p2_spawn = Vector2(32, 0)
room_color_pair = (engine_draw.black, engine_draw.white)
room_box_data = None

while(not engine_io.A.is_just_pressed and not engine_io.B.is_just_pressed):
    engine.tick()

if(selector.selection == 0):
    # Load S-room
    flag_positions = s_room_flag_positions
    room_p1_spawn = s_room_p1_spawn
    room_p2_spawn = s_room_p2_spawn
    room_color_pair = s_room_color_pair
    room_box_data = s_room_box_data
elif(selector.selection == 1):
    # Load Y-room
    flag_positions = y_room_flag_positions
    room_p1_spawn = y_room_p1_spawn
    room_p2_spawn = y_room_p2_spawn
    room_color_pair = y_room_color_pair
    room_box_data = y_room_box_data
elif(selector.selection == 2):
    # Load Springboards
    flag_positions = springboard_room_flag_positions
    room_p1_spawn = springboard_room_p1_spawn
    room_p2_spawn = springboard_room_p2_spawn
    room_color_pair = springboard_room_color_pair
    room_box_data = springboard_room_box_data
elif(selector.selection == 3):
    # Load The Box
    flag_positions = thebox_room_flag_positions
    room_p1_spawn = thebox_room_p1_spawn
    room_p2_spawn = thebox_room_p2_spawn
    room_color_pair = thebox_room_color_pair
    room_box_data = thebox_room_box_data
    pass

splash_text.opacity = 0.0
s_room_text.opacity = 0.0
y_room_text.opacity = 0.0
springboards_room_text.opacity = 0.0
thebox_room_text.opacity = 0.0
two_player_text.opacity = 0.0
under_construction_text.opacity = 0.0
text_crossout.opacity = 0.0
selector.opacity = 0.0

splash_text.mark_destroy_all()
s_room_text.mark_destroy_all()
y_room_text.mark_destroy_all()
springboards_room_text.mark_destroy_all()
thebox_room_text.mark_destroy_all()
two_player_text.mark_destroy_all()
under_construction_text.mark_destroy_all()
text_crossout.mark_destroy_all()
selector.mark_destroy_all()

#player_hitbox = PhysicsRectangle2DNode()

GROUND_ACCEL = 0.2 * 3/2
GROUND_DECEL = 1.2 * 3/2
GROUND_MAX = 2.6 * 3/2
SLIDE_DECEL = 0.02

AIR_ACCEL = 0.06 * 3/2
AIR_DECEL = 0.03 * 3/2
AIR_MAX = 3.5 * 3/2

JUMP_VEL = -2.5
SLIDE_BOOST = 0.6 * 3/2

WALL_FRICTION = 0.1
VELOCITY_EPSILON = 0.1
AI_REACTION_FRAMES = 4

engine_draw.set_background_color(room_color_pair[0])

room_physics_boxes = []
room_draw_boxes = []
room_outline_boxes = []

#@micropython.native
def load_boxes(boxes):
    global room_physics_boxes
    room_physics_boxes.clear()
    for i in range(len(room_box_data)):
        box = PhysicsRectangle2DNode()
        box.outline = False
        box.dynamic = False
        box.position.x = room_box_data[i][0]
        box.position.y = room_box_data[i][1]
        box.width = room_box_data[i][2]
        box.height = room_box_data[i][3]
        box.rotation = room_box_data[i][4]
        box.bounciness = 0.0
        box.friction = 0.0
        box.velocity = Vector2(0, 0)

        draw_box = Rectangle2DNode()
        draw_box.width = box.width
        draw_box.height = box.height
        draw_box.position = Vector2(0, 0)
        draw_box.outline = False
        draw_box.color = room_color_pair[1]
        draw_box.layer = 2
        
        room_draw_boxes.append(draw_box)
        
        box.add_child(draw_box)
        room_physics_boxes.append(box)



class PlayerHitbox(PhysicsRectangle2DNode):
    def __init__(self, player_spr):
        super().__init__(self)
        self.player_spr = player_spr
        self.player_spr.layer = 2
        self.dynamic = True
        #self.solid = True
        self.outline = False
        self.width = 10
        self.height = 14
        self.bounciness = 0.0
        self.friction = 0.0
        self.on_ground = False
        self.wall_dir = 0
        self.hanging = False
        self.can_hang = True
        self.crouching = False
        self.rolling = False
        self.roll_rotation = 0
        self.sees = False

        self.control_left = False
        self.control_right = False
        self.control_up = False
        self.control_down = False
        self.control_A = False
        self.control_B = False

        self.get_input = None
        self.old_pos = None

        #self.density = 1.0
        self.add_child(player_spr)
        #self.add_child(self.hang_detector)
    @micropython.native
    def physics_tick(self, dt):

        if(self.get_input is not None):
            self.get_input(self)

        if(self.old_pos is not None and self.on_ground and abs(self.velocity.x) < VELOCITY_EPSILON):
            if(not (self.control_left or self.control_right)):
                self.position = Vector2(self.old_pos.x, self.old_pos.y)
        self.old_pos = Vector2(self.position.x, self.position.y)

        #self.player_spr.rotation = 0
        if(self.on_ground):
            self.roll_rotation = 0
            if(self.crouching):
                #print("Player crouching")
                self.player_spr.frame_current_y = 2
                self.player_spr.frame_current_x = 2 if abs(self.velocity.x) < VELOCITY_EPSILON else 3
            else:
                #print("Player standing upright")
                self.player_spr.frame_current_y = 0
        else:
            if(not self.rolling):
                self.roll_rotation = 0
                self.player_spr.frame_current_y = 1
                self.dynamic = True
                self.player_spr.playing = True
                if(self.hanging):
                    #print("Player hanging")
                    self.dynamic = False
                    self.player_spr.frame_current_x = 3
                    self.player_spr.frame_current_y = 1
                    self.player_spr.scale.x = 1.0 if self.wall_dir == 1 else -1.0
                    self.velocity.y = 0
                elif(self.wall_dir == 0):
                    #print("Player jump frames")
                    self.player_spr.playing = False
                    if(self.velocity.y < JUMP_VEL / 2):
                        self.player_spr.frame_current_x = 0
                    elif(self.velocity.y > 0):
                        self.player_spr.frame_current_x = 1
                elif(self.velocity.y > 0):
                    #print("Player wall sliding")
                    self.player_spr.playing = False
                    self.player_spr.frame_current_x = 2
                    self.player_spr.scale.x = 1.0 if self.wall_dir == 1 else -1.0
                    self.velocity.y *= (1-WALL_FRICTION) if self.velocity.y > 0 else 1
            else:
                #print("Player rolling")
                self.player_spr.frame_current_y = 2
                self.playing = True

                if(self.player_spr.frame_current_x <= 0):
                    self.roll_rotation = 0
                if(self.player_spr.frame_current_x == 2 or self.roll_rotation == -1):
                    self.player_spr.frame_current_x = 1 if self.roll_rotation != -1 else 0
                    self.roll_rotation += 1
                    if(self.roll_rotation > 3):
                        self.roll_rotation = 0
        if(self.roll_rotation >= 0):
            self.player_spr.rotation = -self.player_spr.scale.x * self.roll_rotation * math.pi/2
        else:
            self.player_spr.rotation = 0

        if(self.control_right and not self.control_left and not self.control_down):
            if(self.on_ground):
                #print("Ground accel")
                self.velocity.x += GROUND_ACCEL
                if(self.velocity.x > GROUND_MAX):
                    self.velocity.x = GROUND_MAX
            elif(not self.control_down):
                #print("Air accel")
                self.velocity.x += AIR_ACCEL
                if(self.velocity.x > VELOCITY_EPSILON and self.hanging):
                    self.velocity.x = VELOCITY_EPSILON
                if(self.velocity.x > AIR_MAX):
                    self.velocity.x = AIR_MAX
        elif(self.velocity.x > 0):
            if(self.on_ground):

                self.velocity.x -= SLIDE_DECEL if self.crouching else GROUND_DECEL
                if(self.velocity.x <= VELOCITY_EPSILON):
                    self.velocity.x = 0
            elif(not self.control_down):
                self.velocity.x -= AIR_DECEL
                if(self.velocity.x <= VELOCITY_EPSILON):
                    self.velocity.x = 0



        if(self.control_left and not self.control_right and not self.control_down):
            if(self.on_ground):
                    self.velocity.x -= GROUND_ACCEL
                    if(self.velocity.x < -GROUND_MAX):
                        self.velocity.x = -GROUND_MAX
            elif(not self.control_down):
                self.velocity.x -= AIR_ACCEL
                if(self.velocity.x < -VELOCITY_EPSILON and self.hanging):
                    self.velocity.x = -VELOCITY_EPSILON
                if(self.velocity.x < -AIR_MAX):
                    self.velocity.x = -AIR_MAX

        elif(self.velocity.x < 0):
            if(self.on_ground):
                self.velocity.x += SLIDE_DECEL if self.crouching else GROUND_DECEL
                if(self.velocity.x >= -VELOCITY_EPSILON):
                    self.velocity.x = 0
            elif(not self.control_down):
                self.velocity.x += AIR_DECEL
                if(self.velocity.x >= VELOCITY_EPSILON):
                    self.velocity.x = 0


        if(self.control_A and not self.control_down):
            #self.control_A = False
            if(self.on_ground or (self.hanging)):
                self.velocity.y = JUMP_VEL
                self.on_ground = False
                self.dynamic = True
                self.hanging = False
                self.wall_dir = 0
            elif(self.wall_dir != 0):
                self.velocity.y = JUMP_VEL * 0.707
                self.velocity.x = JUMP_VEL * 0.707 * -self.wall_dir
                self.wall_dir = 0

        if(self.control_down):
            self.hanging = False
            if(self.on_ground):
                if(not self.crouching):
                    self.height = 12
                    self.position.y += 1
                    if(self.velocity.x > VELOCITY_EPSILON):
                        self.velocity.x += SLIDE_BOOST
                    elif(self.velocity.x < -VELOCITY_EPSILON):
                        self.velocity.x -= SLIDE_BOOST
                self.crouching = True

            else:
                #self.roll_rotation = 0
                if(abs(self.velocity.x) > VELOCITY_EPSILON):
                    #if(not self.rolling):
                    #    self.roll_rotation = -1
                    if(not self.rolling):
                        self.player_spr.frame_current_x = 0
                        self.height = 12
                        self.position.y += 1
                    self.rolling = True

        else:
            if(self.height != 14):
                self.position.y -= 1
            self.height = 14
            if(not self.on_ground):
                self.rolling = False
            self.crouching = False
            #self.old_pos.y -= 2
            #self.height = 16



        #print("Physics pre-tick clearing ground flag")
        if(self.on_ground or abs(self.velocity.x) > VELOCITY_EPSILON):
            self.wall_dir = 0
            self.hanging = False
            self.dynamic = True
            self.can_hang = True

        if((self.velocity.y) > VELOCITY_EPSILON): # Assume player is midair
            self.on_ground = False
            #self.hanging = False
            self.dynamic = True
            self.wall_dir = 0
            self.hanging = False
            self.can_hang = True

    @micropython.native
    def tick(self, dt):

        #print("Velocity is "+str(self.velocity.x)+", "+str(self.velocity.y))

        if(self.wall_dir != 0):
            self.player_spr.scale.x = self.wall_dir
        else:
            if(self.velocity.x < 0):
                self.player_spr.scale.x = -1.0
            elif(self.velocity.x > 0):
                self.player_spr.scale.x = 1.0

        if(abs(self.velocity.x) < 0.1 and self.wall_dir == 0):
            self.player_spr.frame_current_x = 0
            self.player_spr.playing = False
        elif(not self.hanging):
            if(not self.player_spr.playing):
                #self.player_spr.frame_current_x = 1
                self.player_spr.playing = True
            elif(self.player_spr.frame_current_x == 0 and self.player_spr.frame_current_y == 0):
                self.player_spr.frame_current_x = 2

    #self.player_spr.frame_current_y = 0
    #@micropython.native
    @micropython.native
    def on_collide(self, contact):
        if(self.position == player_box.position and contact.node.position == player2_box.position):
            end_game("TAGGED")
        if(not contact.node.solid or contact.node.dynamic):
            return
        if(contact.normal.y < -0.7):
            #print("Ground contact")
            self.on_ground = True
            self.rolling = False
            dot = self.velocity.x * contact.normal.x + self.velocity.y * contact.normal.y
            self.velocity.x -= (dot) * contact.normal.x
            self.velocity.y -= (dot) * contact.normal.y
        elif(abs(contact.normal.y) < math.sin(math.pi/16)):
            #self.velocity.x = 0.0
            #self.wall_dir = 0
            #print("Wall contact")
            self.rolling = False
            dot = self.velocity.x * contact.normal.x + self.velocity.y * contact.normal.y
            self.velocity.x -= (dot) * contact.normal.x
            self.velocity.y -= (dot) * contact.normal.y
            if contact.normal.x >= 0:
                #print("Wall contact!")
                self.wall_dir = 1
                #self.velocity.x = 0.0
            elif contact.normal.x < 0:
                #print("Wall contact!")
                self.wall_dir = -1
                #self.velocity.x = 0.0
        else:
            #print("Other type contact")
            pass
        pass

class PlayerHangDetector(PhysicsRectangle2DNode):
    def __init__(self, player):
        super().__init__(self)
        self.player = player
        self.dynamic = True
        self.solid = False
        self.outline = False
        self.width = 3
        self.height = 4
        self.density = 0.0
        self.bounciness = 0.0
    def tick(self, dt):
        pass
    @micropython.native
    def physics_tick(self, dt):
        self.velocity.x = self.player.velocity.x
        self.velocity.y = self.player.velocity.y
        #self.velocity.x = 0
        #self.velocity.y = 0
        self.position.y = self.player.position.y - 8
        if(self.player.velocity.x < 0):
            self.position.x = self.player.position.x - 5
        elif(self.player.velocity.x > 0):
            self.position.x = self.player.position.x + 5
        else:
            self.position.x = self.player.position.x
            self.position.y = self.player.position.y
        if(not self.player.can_hang):
            self.player.hanging = False

        if(abs(self.velocity.y) > VELOCITY_EPSILON): # Assume player is midair
            self.player.hanging = False
            self.player.can_hang = True
            self.player.dynamic = True
    @micropython.native
    def on_collide(self, contact):
        if(not contact.node.solid or contact.node.dynamic or self.player.control_down):
            return
        #print(dir(contact))
        if(contact.normal.y < -0.7):
            self.player.hanging = True
            self.player.velocity.y = 0.0
            #self.player.dynamic = False
        else:
            self.player.hanging = False
            self.player.can_hang = False
            #self.player.dynamic = True
            #self.player.dynamic = True
            pass
        pass


def random_color(luma):
    u = urandom.random() - 0.5
    v = urandom.random() - 0.5
    cr = luma + 1.1398 * v
    cg = luma - 0.3947 * u - 0.5806 * v
    cb = luma + 2.0321 * u
    cr = cr if cr <= 1.0 else 1.0
    cr = cr if cr >= 0.0 else 0.0
    cg = cg if cg <= 1.0 else 1.0
    cg = cg if cg >= 0.0 else 0.0
    cb = cb if cb <= 1.0 else 1.0
    cb = cb if cb >= 0.0 else 0.0
    c = Color(cr, cg, cb)
    return ((int(c.r * 31) & 31) << 11) | ((int(c.g * 63) & 63) << 5) | (int(c.b * 31) & 31)
    #return c.value

@micropython.native
def set_player_colors(c, c2):
    newtexture = TextureResource("test_runner_frames_3_16bit.bmp", True)
    for i in range(0, newtexture.width * newtexture.height*2):
        if(newtexture.data[i] == 0xFF and newtexture.data[i+1] == 0xFF):
            newtexture.data[i] = int(c) & 0xFF
            newtexture.data[i+1] = int(c >> 8) & 0xFF
        elif(newtexture.data[i] == 0x00 and newtexture.data[i+1] == 0xF8):
            newtexture.data[i] = int(c2) & 0xFF
            newtexture.data[i+1] = int(c2 >> 8) & 0xFF
    return newtexture

WAYPOINT_COUNT = 16

ai_waypoints = [None] * WAYPOINT_COUNT
waypoint_idx = 0

@micropython.native
def player_input(player):
    global ai_waypoints
    global waypoint_idx
    player.control_left = engine_io.LEFT.is_pressed
    player.control_right = engine_io.RIGHT.is_pressed
    player.control_down = engine_io.DOWN.is_pressed
    player.control_A = engine_io.A.is_just_pressed
    closest = get_closest_waypoint(ai_waypoints, player_box.position)
    #closest = ai_waypoints[waypoint_idx-1] if waypoint_idx > 0 else ai_waypoints[WAYPOINT_COUNT-1]
    if(closest is not None):
        dx = closest.position.x - player_box.position.x
        dy = closest.position.y - player_box.position.y
        if(abs(dx) < 4 and abs(dy) < 4):
            closest.position = Vector2(player_box.position.x, player_box.position.y)
        #print("Player closest waypoint is " + str(closest.position.x) + ", " + str(closest.position.y))
        elif(abs(dx) > 8 or abs(dy) > 8 or engine_io.A.is_just_pressed):
            wx = player_box.position.x
            if((engine_io.LEFT.is_pressed) or player_box.velocity.x < 0):
               wx -= 4
            elif((engine_io.RIGHT.is_pressed) or player_box.velocity.x > 0):
               wx += 4
            if(ai_waypoints[waypoint_idx] is None):
                ai_waypoints[waypoint_idx] = Waypoint(Vector2(wx, player_box.position.y))
                pass
            else:
                #ai_waypoints[waypoint_idx].mark_destroy()
                pass
            ai_waypoints[waypoint_idx].position = Vector2(wx, player_box.position.y)
            ai_waypoints[waypoint_idx].left = player.control_left
            ai_waypoints[waypoint_idx].right = player.control_right
            ai_waypoints[waypoint_idx].down = player.control_down
            ai_waypoints[waypoint_idx].jump = player.control_A
            closest.next = ai_waypoints[waypoint_idx]
            ai_waypoints[waypoint_idx].prev = closest
            waypoint_idx = waypoint_idx + 1 if waypoint_idx < WAYPOINT_COUNT-1 else 0
    else:

        ai_waypoints[waypoint_idx] = Waypoint(Vector2(player_box.position.x, player_box.position.y))
        #waypoint_boxes[waypoint_idx] = WaypointSprite(Vector2(player_box.position.x, player_box.position.y))
        waypoint_idx = waypoint_idx + 1 if waypoint_idx < WAYPOINT_COUNT-1 else 0
    #if(engine_io.check_just_pressed(engine_io.B)):


player = Sprite2DNode()
#player.texture = set_player_colors(urandom.randrange(65536), urandom.randrange(65536))
player.texture = set_player_colors(random_color(0.6), random_color(0.8))
player.frame_count_x = 4
player.frame_current_x = 0
player.frame_count_y = 3
player.frame_current_y  = 0
player.transparent_color = Color(0x07e0)
player.loop = False
player.playing = True
VELOCITY_EPSILON = 0.1

player.fps = 8
player.scale = Vector2(1.0, 1.0)


class Waypoint():
    def __init__(self, pos):
        #super().__init__(self)
        self.position = pos
        self.next = None
        self.prev = None
        self.left = False
        self.right = False
        self.down = False
        self.up = False
        self.jump = False
        self.radius = 1

    def tick(self, dt):
        pass
#ai_waypoints.append(Vector2(0, 0))

player_box = PlayerHitbox(player)
player_box.position = room_p1_spawn
player_box.get_input = player_input

player_hang_detector = PlayerHangDetector(player_box)

#player_box.add_child(cam)

@micropython.native
def get_closest_waypoint(waypoints, v):
    closest = None
    dist = math.inf
    for w in waypoints:
        if(w is not None):
                # if(w.position.x != v.x or w.position.y != v.y):
            dx = w.position.x - v.x
            dy = w.position.y - v.y
            if(dx*dx + dy*dy < dist):
                dist = dx*dx + dy*dy
                closest = w
    return closest

AI_TARGET = None


class AISight(PhysicsRectangle2DNode):
    def __init__(self, player):
        super().__init__(self)
        self.outline = False
        self.dynamic = True
        self.solid = False
        self.player = player
        self.hit = False
        self.dist2 = 0
        self.width = 1
        #player.add_child(self)
    @micropython.native
    def physics_tick(self, dt):
        if(self.hit):
            #print("Obstructed")
            pass
        else:
            #print("Clear line of sight!")
            self.player.sees = True
            pass
        #     pass
        # self.height = 2
        # self.width = math.sqrt(dx*dx+dy*dy)
        # self.rotation = math.atan2(-dy, dx)
        # self.velocity.x = player_box.velocity.x
        # self.velocity.y = player_box.velocity.y
        # self.position.x = player_box.position.x + dx/2
        # self.position.y = player_box.position.y + dy/2
        self.velocity = Vector2(0, 0)
        self.adjust_from_to(self.player.position, player_box.position)
        self.dist2 = self.height
        self.hit = False

    @micropython.native
    def on_collide(self, contact):
        #print("Line-of-sight hit")
        if(contact.node.position == player_box.position):
            #print("Hit player")
            pass
        elif(contact.node.position == player_hang_detector.position or contact.node.position == player2_hang_detector.position):
            #print("Sight obstructed by hang detector at "+str(contact.node.position.x)+", "+str(contact.node.position.y))
            pass
        else:
            #print("Sight obstructed at "+str(contact.node.position.x)+", "+str(contact.node.position.y))
            self.hit = True
        pass

ai_reacting = 0
class Countdown(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.start = utime.ticks_us()
        self.font = font
        self.opacity = 1.
        self.text = str(10)
        self.layer = 7
        self.top = 15.0
        self.position = Vector2(-20, -24)
        self.running = False
        self.color = engine_draw.black
    def restart(self, top):
        self.start = utime.ticks_us()
        self.top = top
    def remaining(self):
        return self.top - round(utime.ticks_diff(utime.ticks_us(), self.start)/10000.0)*0.01
    def tick(self, dt):
        if(not self.running):
            self.start = utime.ticks_us()
        time_counter = self.top - round(utime.ticks_diff(utime.ticks_us(), self.start)/10000.0)*0.01
        count = "{:10.2f}".format(time_counter) if self.running else "{:10.2f}".format(self.top)
        self.text = str(count)
        if(self.remaining() < 0 and self.running):
            end_game("TIME")
        pass

class FxText(Text2DNode):
    def __init__(self):
        super().__init__(self)
        self.font = font
        self.opacity = 1.
        self.pos_tween = Tween()
        self.opacity_tween = Tween()
        self.position = Vector2(0, -24)
        self.text = " "
        self.layer = 7
        self.color = engine_draw.black
    def animate(self, text):
        self.text = text
        self.pos_tween.start(self, "position", Vector2(0, -24), Vector2(0, -12), 500, 1.0, ONE_SHOT, EASE_QUAD_IN)
        self.opacity_tween.start(self, "opacity", 1., 0., 1500, 1.0, ONE_SHOT, EASE_QUAD_IN)

countdown = Countdown()
fx = FxText()

flags_held = 0
game_running = True
end_text = Text2DNode()
end_text.opacity = 0.

def end_game(text):
    global game_running
    if(not game_running):
        return
    global end_text
    global Countdown
    game_running = False

    countdown.top = countdown.remaining()
    countdown.running = False

    end_text.position = Vector2(0,0)
    end_text.font = font
    end_text.text = text
    end_text.opacity = 1.
    end_text.layer = 7
    end_text.color = engine_draw.black


class Flag(PhysicsCircle2DNode):
    def __init__(self, pos):
        super().__init__(self)
        self.radius = 2.5
        self.position = pos
        self.dynamic = False
        self.solid = False
        self.outline = True

    def physics_tick(self, dt):
        global ai_awake
        global countdown
        global flags_held
        dx = player_box.position.x - self.position.x
        dy = player_box.position.y - self.position.y
        if(abs(dx) < 4 and abs(dy) < 4):
            ai_awake = True
            flags_held += 1
            if(flags_held == 1):
                countdown.restart(15.0)
                countdown.running = True
                fx.animate("GO GO GO!!")
            elif(flags_held >= (len(flag_positions))):
                end_game("FLAGGED")
            else:
                if(countdown.remaining() > 8.0):
                    fx.animate("-3.00!!")
                    countdown.top -= 3.
                else:
                    fx.animate("HURRY!!")
            self.outline = False
            self.mark_destroy()
        pass
    def on_collide(self, contact):
        #if(contact.node.position == player_box.position):
            #ai_awake = True
            #pass
        pass

flags = []

for i in range(len(flag_positions)):
    flags.append(Flag(flag_positions[i]))

ai_awake = False

@micropython.native
def ai_input(player):
    global player_box
    global AI_TARGET
    global ai_reacting
    waypoint_jump = False
    #ai_waypoints.append(player_box.position)
    dx = None
    dy = None
    if(ai_reacting != 0):
        ai_reacting -= 1
        return
    else:
        ai_reacting = AI_REACTION_FRAMES
    if(not ai_awake):
        return
    if(not player.sees):
        #print("Following trail")
        if(AI_TARGET is None):
            AI_TARGET = get_closest_waypoint(ai_waypoints, player.position)

        dx = player.position.x - AI_TARGET.position.x
        dy = player.position.y - AI_TARGET.position.y

        closest = get_closest_waypoint(ai_waypoints, AI_TARGET.position)

        if(abs(dx) < 4 and abs(dy) < 4):
            # Player close to current target, switch to next
            #print("Switching to next waypoint")
            waypoint_jump = AI_TARGET.jump
            tmp = AI_TARGET.next
            AI_TARGET = AI_TARGET.next
            if(AI_TARGET is not None):
                dx = player.position.x - AI_TARGET.position.x
                dy = player.position.y - AI_TARGET.position.y
        elif(abs(dx) > 16 or abs(dy) > 16):
            # Give up and pick up the trail
            #print("AI far from target, picking up the trail")
            AI_TARGET = get_closest_waypoint(ai_waypoints, player.position)
            if(AI_TARGET is not None):
                dx = player.position.x - AI_TARGET.position.x
                dy = player.position.y - AI_TARGET.position.y
        else:
            #print("Pathfinding to "+str(AI_TARGET.position.x)+", "+str(AI_TARGET.position.y))
            pass
            # Still pathfinding
    else:
        player.sees = False
        #print("AI targeting player")
        AI_TARGET = get_closest_waypoint(ai_waypoints, player_box.position)
        dx = player.position.x - player_box.position.x
        dy = player.position.y - player_box.position.y


    player.control_down = True if ((dy < -8 and not player.control_left and not player.control_right) or ((dy < 0) and player.hanging)) else False
    player.control_A = True if ((dy > 4 and abs(dy) > abs(dx)) or ((player.wall_dir * dx) < 0 and dy > 0) or (player.hanging and dy > 0)) else False
    player.control_left = True if (dx > 0) else False
    player.control_right = True if (dx < 0) else False

player2 = Sprite2DNode()
player2_box = PlayerHitbox(player2)
player2_hang_detector = PlayerHangDetector(player2_box)
ai_sight = AISight(player2_box)

player2.texture = set_player_colors(random_color(0.6), random_color(0.8))
player2.frame_count_x = 4
player2.frame_current_x = 0
player2.frame_count_y = 3
player2.frame_current_y  = 0
player2.transparent_color = Color(0x07e0)
player2.loop = False
player2.playing = True
player2.fps = 8
player2.scale = Vector2(1.0, 1.0)

player2_box.get_input = ai_input
player2_box.position = room_p2_spawn

load_boxes(room_box_data)

#engine.start()

while(game_running):
    if(engine.tick()):
        # player.frame_current_x = 3
        # player.frame_current_y = 1
        cam.position = Vector2(0, 0)
        print(engine.get_running_fps())
        #print(gc.mem_free())
        #print("Flags held: "+str(flags_held))
        pass

start = utime.ticks_us()
while(utime.ticks_diff(utime.ticks_us(), start) < 3*1000000):
   if(engine.tick()):
       #print("Engine tick inside end loop")
       pass
