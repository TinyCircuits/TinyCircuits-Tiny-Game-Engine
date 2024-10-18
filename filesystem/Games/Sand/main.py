import engine_main

import engine
import engine_io
import engine_draw
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource

import framebuf
import random
import gc
import math

W = const(130)
H = const(130)

# Particle Format (8-bit)
#   IIISDDCC
#   C: Color (variants)
#   D: Density
#   S: Static
#   I: ID

P_C = const(0b00000011)     # Color bit mask
P_D = const(0b00001100)     # Density bit mask
P_ID = const(0b11100000)     # ID bit mask

# Density values
P_D_GAS = const(0 << 2)
P_D_LIQUID = const(1 << 2)
P_D_SOLID = const(2 << 2)

# STATIC bit flag
P_S_STATIC = const(1 << 4)

# Particle definitions
#   - Each type of particle consists of an ID, static flag, density value, and color value
#   - By using bitwise OR, they can be composed by combining each category value
P_AIR = const((0 << 5) | P_S_STATIC | P_D_GAS)
P_SAND = const((1 << 5) | P_D_SOLID)
P_WATER = const((2 << 5) | P_D_LIQUID)
P_WALL = const((3 << 5) | P_S_STATIC | P_D_SOLID)

# Cycle order for picking an element (B button)
Picks = [P_SAND, P_WATER, P_WALL]

# State Format (8-bit)
#   000000BM
#   M: MOVED
#   B: BIAS

S_M = const(0b00000001)  # MOVED bit mask
S_B = const(0b00000010)  # BIAS bit mask

# Set/Reset MOVED bit flag
S_M_MOVED = const(1)
S_M_INV_MOVED = const(255 - S_M_MOVED)

# Set/Reset BIAS bit flag
S_B_BIAS = const(1 << 1)
S_B_INV_BIAS = const(255 - S_B_BIAS)

# Buffers that contain each pixel on the screen, plus some extra along the edges
particles = bytearray(W*H)  # Particle type (air, sand, water, wall)
state = bytearray(W*H)      # Particle state (moved, bias)

# Setup empty screen full of air, with the extra along the edges as walls
for px in range(W):
    particles[px] = P_WALL
    particles[W*(H-1)+px] = P_WALL
for py in range(H):
    particles[py*W] = P_WALL
    particles[py*W+(W-1)] = P_WALL
for py in range(1, H-1):
    for px in range(1, W-1):
        particles[py*W+px] = P_AIR


@micropython.viper
def physics():
    # Viper pointers for quick access to the buffers
    pa = ptr8(particles)
    sa = ptr8(state)

    for py in range(1, H-1):
        for i in range(W):
            # Every other row, reverse processing order to reduce left-right bias
            # (this trick may not be needed anymore, since I added a bias state?)
            if py & 0b1:
                px = 1 + i
            else:
                px = (W-2) - i

            mi = W * py + px   # middle index (particle/state being processed)

            mmp, mms = pa[mi], sa[mi]   # middle particle, middle state
            mmpd = mmp & P_D            # middle particle density
            mmsb = mms & S_B            # middle state bias flag

            # Skip processing this particle if flagged static (wall) or already moved this frame
            if mmp & P_S_STATIC or mms & S_M_MOVED:
                continue

            # Use bias flag to determine priority (1, 2) when deciding middle sideways movement
            if mmpd == P_D_LIQUID:
                if mmsb:
                    msi1, msi2 = mi-1, mi+1     # middle side index 1 & 2
                else:
                    msi1, msi2 = mi+1, mi-1     # middle side index 1 & 2
                msp1, mss1 = pa[msi1], sa[msi1]  # m side 1 particle & state
                msp2, mss2 = pa[msi2], sa[msi2]  # m side 2 particle & state

            di = mi + H                # down index (pixel below)
            dmp, dms = pa[di], sa[di]   # down particle, down state

            # Use bias flag to determine priority (1, 2) when deciding down sideways movement
            if mmsb:
                dsi1, dsi2 = di-1, di+1     # down side index 1 & 2
            else:
                dsi1, dsi2 = di+1, di-1     # down side index 1 & 2
            dsp2, dss2 = pa[dsi2], sa[dsi2]  # d side 1 particle & state
            dsp1, dss1 = pa[dsi1], sa[dsi1]  # d side 2 particle & state

            # If down particle hasn't moved yet and has lighter density
            if not dms & S_M_MOVED and dmp & P_D < mmpd:
                si, sb = di, mmsb ^ S_B_BIAS    # Swap index & bias (flip)
            # If down side 1 particle hasn't moved yet and has lighter density
            elif not dss1 & S_M_MOVED and dsp1 & P_D < mmpd:
                si, sb = dsi1, mmsb             # Swap index & bias
            # If down side 2 particle hasn't moved yet and has lighter density
            elif not dss2 & S_M_MOVED and dsp2 & P_D < mmpd:
                si, sb = dsi2, mmsb ^ S_B_BIAS  # Swap index & bias (flip)
            # If I am liquid and middle side 1 particle hasn't moved yet and has lighter density
            elif mmpd == P_D_LIQUID and not mss1 & S_M_MOVED and msp1 & P_D < mmpd:
                si, sb = msi1, mmsb             # Swap index & bias
            # If I am liquid and middle side 2 particle hasn't moved yet and has lighter density
            elif mmpd == P_D_LIQUID and not mss2 & S_M_MOVED and msp2 & P_D < mmpd:
                si, sb = msi2, mmsb ^ S_B_BIAS  # Swap index & bias (flip)
            # If no swaps are found, skip to next particle
            else:
                continue

            # Swap particles
            pa[mi], pa[si] = pa[si], pa[mi]

            # Flag both have moved
            sa[mi] |= S_M_MOVED
            sa[si] |= S_M_MOVED

            # Set/Reset bias flag
            if sb:
                sa[si] |= S_B_BIAS
            else:
                sa[si] &= S_B_INV_BIAS

    # Reset moved flag for all to be ready for next frame
    for i in range(W*H):
        sa[i] &= S_M_INV_MOVED


palettes_raw = [
    0x2965, 0x2965, 0x2965, 0x2965,  # AIR
    0xe736, 0xd6d5, 0xdef6, 0xf7b8,  # SAND
    0x63d7, 0x63d7, 0x63d7, 0x63d7,  # WATER
    0xa513, 0xa4f2, 0x94b1, 0x9490,  # WALL
    0, 0, 0, 0,  # 5
    0, 0, 0, 0,  # 6
    0, 0, 0, 0,  # 7
    0, 0, 0, 0,  # 8
]
palettes = bytearray([((v >> (8*i)) & 0xFF)
                     for v in palettes_raw for i in range(2)])


@micropython.native
def randomColor(pick):
    c = random.randrange(0, 4)
    return (pick & 0b11111100) | c


@micropython.viper
def render():

    # Viper pointers for quick access to the buffers
    buf = ptr16(engine_draw.back_fb_data())
    pa = ptr8(particles)
    pal = ptr16(palettes)

    o = 0   # screen index
    for py in range(1, H-1):
        for px in range(1, W-1):
            p = pa[W*py+px]    # particle
            pal_row = p >> 5
            pal_col = p & P_C
            c = pal[pal_row*4+pal_col]
            buf[o] = c
            o += 1


shapes = {
    "square": {
        "mass": 1,
        "k": 0.25,
        "friction": 0.25,
        "vertices": {
            "p1": (10, 15),
            "p2": (20, 10),
            "p3": (25, 20),
            "p4": (15, 25)
        },
        "springs": [
            ("p1", "p2", True),
            ("p2", "p3", True),
            ("p3", "p4", True),
            ("p4", "p1", True),
            ("p1", "p3", False),
            ("p2", "p4", False),
        ]
    },
}

vertices = []
springs = []
friction = 1


def loadShape(key):
    global vertices, springs, friction

    vertices = []
    springs = []

    gc.collect()

    shapeData = shapes[key]

    m = shapeData["mass"]
    k = shapeData["k"]
    friction = shapeData["friction"]

    vertexLookup = {}
    vertexData = shapeData["vertices"]
    for vk in vertexData:
        x, y = vertexData[vk]
        vertex = Vertex(x, y, 0, 0, m)
        vertexLookup[vk] = vertex
        vertices.append(vertex)

    for vk1, vk2, visible in shapeData["springs"]:
        v1 = vertexLookup[vk1]
        v2 = vertexLookup[vk2]
        d = dist(v1, v2)
        springs.append(Spring(v1, v2, d, k, visible))

    vertexLookup = None
    gc.collect()


GRAV = const(0.1)
BOUNCE = const(0.25)
DAMP = const(0.1)
MAX_SPEED = const(2)

# Default gravity
DEFAULT_GRAV_X = 0
DEFAULT_GRAV_Y = GRAV

gravX = DEFAULT_GRAV_X
gravY = DEFAULT_GRAV_Y


class Vertex:
    def __init__(self, x, y, dx, dy, mass):
        self.x = x
        self.y = y
        self.dx = dx
        self.dy = dy
        self.mass = mass


class Spring:
    def __init__(self, v1, v2, d, k, visible):
        self.v1 = v1
        self.v2 = v2
        self.d = d
        self.k = k
        self.visible = visible


def dist(v1, v2):
    dx = v2.x-v1.x
    dy = v2.y-v1.y
    return math.sqrt(dx*dx + dy*dy)


def shapePhysics():

    for v in vertices:
        px = int(v.x)+1
        py = int(v.y)+1
        p = particles[py*W+px]

        bounce_top = 0
        bounce_bottom = 127
        bounce_left = 0
        bounce_right = 127

        # float up through sand/wall (solids)
        if (p & P_D) == P_D_SOLID:
            bounce_bottom = v.y - 1

        # TODO buoyancy in water?

        cx, cy = int(v.x), int(v.y)

        v.x += v.dx
        v.y += v.dy

        cx2, cy2 = int(v.x), int(v.y)

        # bounce off solids - need to check pixel by pixel
        if cx != cx2 or cy != cy2:
            csx = 1 if cx2 >= cx else -1
            csy = 1 if cy2 >= cy else -1
            while cx != cx2 or cy != cy2:
                if cx != cx2:
                    cx += csx
                if cy != cy2:
                    cy += csy
                if cx < 0 or cx > 127 or cy < 0 or cy > 127:
                    break
                i = (cy+1)*W+(cx+1)
                p2 = particles[i]
                # If we hit solid, check adjacent for bounces
                if (p2 & P_D) == P_D_SOLID:
                    # Horizontal
                    sh = ((particles[i-csx] & P_D) == P_D_SOLID)
                    # Vertical
                    sv = ((particles[i-csy*W] & P_D) == P_D_SOLID)
                    # Diagonal
                    sd = ((particles[i-csy*W-csx] & P_D) == P_D_SOLID)

                    # Bounce Up/Down
                    if sh and (not sv or not sd):
                        if csy > 0:
                            bounce_bottom = cy
                        else:
                            bounce_top = cy
                    # Bounce Left/Right
                    if sv and (not sh or not sd):
                        if csx > 0:
                            bounce_right = cx
                        else:
                            bounce_left = cx



        v.dx += gravX
        v.dy += gravY

        v.dx = max(-MAX_SPEED, min(MAX_SPEED, v.dx))
        v.dy = max(-MAX_SPEED, min(MAX_SPEED, v.dy))

        if v.x < bounce_left:
            v.x = bounce_left
            v.dx = abs(v.dx) * BOUNCE
            v.dy *= friction
        elif v.x > bounce_right:
            v.x = bounce_right
            v.dx = -abs(v.dx) * BOUNCE
            v.dy *= friction
        if v.y < bounce_top:
            v.y = bounce_top
            v.dy = abs(v.dy) * BOUNCE
            v.dx *= friction
        elif v.y > bounce_bottom:
            v.y = bounce_bottom
            v.dy = -abs(v.dy) * BOUNCE
            v.dx *= friction

    for s in springs:
        dx = s.v2.x - s.v1.x
        dy = s.v2.y - s.v1.y
        mag = math.sqrt(dx * dx + dy * dy)

        f = (mag - s.d) * s.k

        if mag == 0:
            continue

        dx /= mag  # Normalize
        dy /= mag  # Normalize

        fx = f * dx - s.v1.dx * DAMP
        fy = f * dy - s.v1.dy * DAMP

        s.v1.dx += fx / s.v1.mass
        s.v1.dy += fy / s.v1.mass
        s.v2.dx -= fx / s.v2.mass
        s.v2.dy -= fy / s.v2.mass


cpx = W//2     # Cursor X
cpy = H//3     # Cursor Y
cdx = 0        # Cursor X Speed
cdy = 0        # Cursor Y Speed
cpick = 0   # Cursor particle selection

CURSOR_ACCEL = const(0.2)
CURSOR_MAX_SPEED = const(2)
CURSOR_DRAG = const(0.8)

loadShape("square")

engine.fps_limit(60)

while True:
    if engine.tick():

        # Cursor Inputs
        cax, cay = 0, 0
        if engine_io.UP.is_pressed:
            cay -= CURSOR_ACCEL
        if engine_io.DOWN.is_pressed:
            cay += CURSOR_ACCEL
        if engine_io.LEFT.is_pressed:
            cax -= CURSOR_ACCEL
        if engine_io.RIGHT.is_pressed:
            cax += CURSOR_ACCEL

        # Cursor Motion
        if cax != 0:
            cdx = max(-CURSOR_MAX_SPEED, min(CURSOR_MAX_SPEED, cdx + cax))
        else:
            cdx *= CURSOR_DRAG
        if cay != 0:
            cdy = max(-CURSOR_MAX_SPEED, min(CURSOR_MAX_SPEED, cdy + cay))
        else:
            cdy *= CURSOR_DRAG
        cpx = max(1, min(W-3, cpx + cdx))
        cpy = max(1, min(H-3, cpy + cdy))

        # Cycle particle selection
        if engine_io.RB.is_just_pressed:
            cpick = (cpick + 1) % len(Picks)
        if engine_io.LB.is_just_pressed:
            cpick = (cpick + len(Picks) - 1) % len(Picks)

        # Rounded to nearest pixel
        cx = int(cpx)
        cy = int(cpy)

        # Draw particles near cursor
        if engine_io.A.is_pressed:
            p = Picks[cpick]
            for y in range(3):
                for x in range(3):
                    particles[(cy+y)*W+cx+x] = randomColor(p)

        # Remove particles near cursor
        if engine_io.B.is_pressed:
            for y in range(3):
                for x in range(3):
                    particles[(cy+y)*W+cx+x] = P_AIR

        # Once each frame, update physics and render particles to display
        physics()
        render()

        # Shape Physics
        shapePhysics()

        fb = engine_draw.back_fb()

        # Overlay shape
        for spring in springs:
            if not spring.visible:
                continue
            v1 = spring.v1
            v2 = spring.v2
            fb.line(int(v1.x), int(v1.y), int(v2.x), int(v2.y), 0xb082)

        # Overlay cursor
        fb.line(cx+1, cy+1, cx+3, cy+3, 0x0000)
        fb.rect(cx+3, cy+3, 5, 5, palettes_raw[4*(Picks[cpick] >> 5)], True)
        fb.rect(cx+3, cy+3, 5, 5, 0x0000, False)
