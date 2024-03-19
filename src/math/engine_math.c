#include "engine_math.h"
#include "trig_tables.h"
#include "math.h"
#include "debug/debug_print.h"


float engine_math_dot_product(float x0, float y0, float x1, float y1){
    return x0*x1 + y0*y1;
}


float engine_math_cross_product(float x0, float y0, float x1, float y1){
    return x0*y1 - y0*x1;
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


void engine_math_swap(float *val0, float *val1){
    float temp = *val0;
    *val0 = *val1;
    *val1 = temp;
}


// https://stackoverflow.com/a/5989243
bool engine_math_compare_floats(float value0, float value1){
    return fabsf(value0 - value1) < EPSILON;
}


// https://stackoverflow.com/a/2339510
float engine_math_angle_between(float px0, float py0, float px1, float py1){
    return atan2f(py0 - py1, px0 - px1);
}


float engine_math_distance_between(float px0, float py0, float px1, float py1){
    float dx = px1 - px0;
    float dy = py1 - py0;
    return sqrtf((dx*dx) + (dy*dy));
}


uint32_t engine_math_2d_to_1d_index(uint32_t x, uint32_t y, uint32_t width){
    return y * width + x;
}


void engine_math_2d_midpoint(float x0, float y0, float x1, float y1, float *mx, float *my){
    *mx = (x0 + x1) / 2.0f;
    *my = (y0 + y1) / 2.0f;
}


// https://stackoverflow.com/a/1968345
// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#:~:text=Given%20two%20points%20on%20each%20line%20segment
bool engine_math_2d_do_segments_intersect(float line_0_start_x, float line_0_start_y, float line_0_end_x, float line_0_end_y, 
                                          float line_1_start_x, float line_1_start_y, float line_1_end_x, float line_1_end_y,
                                          float *intersect_x, float *intersect_y){

    float segment_0_x = line_0_end_x - line_0_start_x;
    float segment_0_y = line_0_end_y - line_0_start_y;

    float segment_1_x = line_1_end_x - line_1_start_x;
    float segment_1_y = line_1_end_y - line_1_start_y;

    float denominator = -segment_1_x * segment_0_y + segment_0_x * segment_1_y;

    if(engine_math_compare_floats(denominator, 0.0f)){
        return false;
    }

    float s_numerator = -segment_0_y * (line_0_start_x - line_1_start_x) + segment_0_x * (line_0_start_y - line_1_start_y);
    float t_numerator =  segment_1_x * (line_0_start_y - line_1_start_y) - segment_1_y * (line_0_start_x - line_1_start_x);

    float s = s_numerator / denominator;
    float t = t_numerator / denominator;

    if(s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f){
        if(intersect_x != NULL) *intersect_x = line_0_start_x + (t * segment_0_x);
        if(intersect_y != NULL) *intersect_y = line_0_start_y + (t * segment_0_y);
        return true;
    }

    return false;
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