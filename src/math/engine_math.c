#include "engine_math.h"
#include "trig_tables.h"
#include "math.h"
#include "debug/debug_print.h"


float engine_math_dot_product(float x0, float y0, float x1, float y1){
    return x0*x1 + y0*y1;
}


void engine_math_normalize(float *vx, float *vy){
    const float factor = 1.0 / sqrtf((*vx) * (*vx) + (*vy) * (*vy));
    *vx = (*vx) * factor;
    *vy = (*vy) * factor;
}


// https://stackoverflow.com/a/16659263
float engine_math_clamp(float value, float min, float max){
    const float t = value < min ? min : value;
    return t > max ? max : t;
}


// https://stackoverflow.com/a/5989243
bool engine_math_compare_floats(float value0, float value1){
    return fabsf(value0 - value1) < EPSILON;
}


// https://stackoverflow.com/a/2339510
float engine_math_angle_between(float px0, float py0, float px1, float py1){
    return atan2f(py0 - py1, px0 - px1);
}


uint32_t engine_math_2d_to_1d_index(uint32_t x, uint32_t y, uint32_t width){
    return y * width + x;
}


// https://stackoverflow.com/a/22491252
void engine_math_rotate_point(float *px, float *py, float cx, float cy, float angle_radians){
    float x_centered = *px - cx;
    float y_centered = *py - cy;

    // Store these so it doesn't have to do it 2x
    float cos_angle = cosf(angle_radians);
    float sin_angle = sinf(angle_radians);

    float x_center_rotated = x_centered * cos_angle - y_centered * -sin_angle;
    float y_center_rotated = x_centered * -sin_angle + y_centered * cos_angle;

    *px = x_center_rotated + cx;
    *py = y_center_rotated + cy;
}


// https://math.stackexchange.com/a/5808
// https://math.stackexchange.com/a/234631
void engine_math_scale_point(float *px, float *py, float cx, float cy, float scale){
    *px = scale * ((*px)-cx);
    *py = scale * ((*py)-cy);
}