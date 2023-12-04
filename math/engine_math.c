#include "engine_math.h"
#include "math.h"

// https://stackoverflow.com/a/22491252
void engine_math_rotate_point(float *px, float *py, float cx, float cy, float angle_radians){
    float x_centered = *px - cx;
    float y_centered = *py - cy;

    // Store these so it doesn't have to be 2x
    float cos_angle = cosf(angle_radians);
    float sin_angle = sinf(angle_radians);

    float x_center_rotated = x_centered * cos_angle - y_centered * sin_angle;
    float y_center_rotated = x_centered * sin_angle + y_centered * cos_angle;

    *px = x_center_rotated + cx;
    *py = y_center_rotated + cy;
}