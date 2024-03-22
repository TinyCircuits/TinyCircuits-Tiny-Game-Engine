import matplotlib.pyplot as plt
import math

SCREEN_WIDTH = 128
SCREEN_HEIGHT = 128

rotation_radians = math.pi / 4
rotation_radians = 0
window_width = 32
window_height = 32
pixels_stride = 32
center_x = 64
center_y = 64

sin_angle = math.sin(rotation_radians)
cos_angle = math.cos(rotation_radians)

midX = window_width / 2.0
midY = window_height / 2.0

center_pixel_offset = int((center_y - midY) * SCREEN_WIDTH + (center_x - midX))
print(center_pixel_offset)
dest_offset = int(center_pixel_offset)
next_dest_row_offset = int(SCREEN_WIDTH - window_width)

collected_x = []
collected_y = []
collected = []

for j in range(window_height):
    deltaY = j - midY
    deltaX = 0 - midX

    x = midX + deltaX * cos_angle + deltaY * sin_angle
    y = midY - deltaX * sin_angle + deltaY * cos_angle

    for j in range(window_width):
        rotX = int(x)
        rotY = int(y)

        if (rotX >= 0 and rotX < window_width) and (rotY >= 0 and rotY < window_height):
            src_offset = rotY * pixels_stride + rotX

            collected_x.append(rotX)
            collected_y.append(rotY)
            # collected.append(dest_offset)

        x += cos_angle
        y -= sin_angle
        dest_offset += 1

    dest_offset += next_dest_row_offset

plt.scatter(collected_x, collected_y)
# plt.plot(collected)
plt.axis('square')
plt.show()