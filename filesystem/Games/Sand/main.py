import engine_main

import engine
import engine_io
import engine_draw
from engine_nodes import Sprite2DNode, CameraNode
from engine_resources import TextureResource

import framebuf
import random

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
P_I = const(0b11100000)     # ID bit mask

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
    0xef9f,0xef9f,0xef9f,0xef9f,# AIR
    0xe736,0xd6d5,0xdef6,0xf7b8,# SAND
    0x63d7,0x63d7,0x7c9a,0x63d6,# WATER
    0xad75,0xa4f2,0xb574,0xd6ba,# WALL
    0,0,0,0,# 5
    0,0,0,0,# 6
    0,0,0,0,# 7
    0,0,0,0,# 8
]
palettes = bytearray([((v>>(8*i))&0xFF) for v in palettes_raw for i in range(2)])
print(palettes)

@micropython.native
def randomColor(pick):
    c = random.randrange(0,4)
    return (pick & 0b11111100) | c

# Render particles directly into display buffer
@micropython.viper
def render():
    # Viper pointers for quick access to the buffers
    buf = ptr16(engine_draw.front_fb_data)
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

# texture = TextureResource("blank.bmp", True)
# fb = framebuf.FrameBuffer(texture.data, texture.width, texture.height, framebuf.RGB565)

# pixel_buf = texture.data

# spr = Sprite2DNode(texture=texture)
# cam = CameraNode()

cx = W//2     # Cursor X
cy = H//3     # Cursor Y
cpick = 0   # Cursor particle selection

engine.set_fps_limit(60)

while True:
    if engine.tick():
        # Move cursor
        if engine_io.check_pressed(engine_io.DPAD_UP) and cy > 2:
            cy -= 1
        if engine_io.check_pressed(engine_io.DPAD_DOWN) and cy < (H-2):
            cy += 1
        if engine_io.check_pressed(engine_io.DPAD_LEFT) and cx > 2:
            cx -= 1
        if engine_io.check_pressed(engine_io.DPAD_RIGHT) and cx < (W-2):
            cx += 1

        # Cycle particle selection
        if engine_io.check_just_pressed(engine_io.BUMPER_RIGHT):
            cpick = (cpick + 1) % len(Picks)
        if engine_io.check_just_pressed(engine_io.BUMPER_LEFT):
            cpick = (cpick + len(Picks) - 1) % len(Picks)

        # Draw particles near cursor
        if engine_io.check_pressed(engine_io.A):
            particles[cy * W + cx] = randomColor(Picks[cpick])
            particles[cy * W + cx - 1] = randomColor(Picks[cpick])
            particles[cy * W + cx - W] = randomColor(Picks[cpick])
            particles[cy * W + cx - (W+1)] = randomColor(Picks[cpick])

        # Remove particles near cursor
        if engine_io.check_pressed(engine_io.B):
            particles[cy * W + cx] = P_AIR
            particles[cy * W + cx - 1] = P_AIR
            particles[cy * W + cx - W] = P_AIR
            particles[cy * W + cx - (W+1)] = P_AIR

        # Once each frame, update physics and render particles to display
        physics()
        render()

        # Overlay cursor
        fb = engine_draw.front_fb
        fb.line(cx, cy, cx+2, cy+2, 0xFFFF)
        fb.rect(cx+2, cy+2, 4, 4, palettes_raw[4*(Picks[cpick]>>5)], True)
        fb.rect(cx+2, cy+2, 4, 4, 0xFFFF, False)
