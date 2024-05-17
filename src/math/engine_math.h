#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include <stdint.h>
#include <stdbool.h>
#include "utility/engine_defines.h"

#define PI      3.14159265358979323846f
#define HALF_PI 1.57079632679489661923f
#define TWICE_PI 6.28318530717958647692f
#define RAD2DEG (180.0f/PI)
#define DEG2RAD (PI/180.0f)
#define EPSILON 1e-6

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

// https://stackoverflow.com/a/5441379
#define min3(a,b,c)         min(min(a, b), c)
#define max3(a,b,c)         max(max(a, b), c) 

float engine_math_dot_product(float x0, float y0, float x1, float y1);

float engine_math_cross_product_v_v(float x0, float y0, float x1, float y1);
void engine_math_3d_cross_product_v_v(float ax, float ay, float az, float bx, float by, float bz, float *out_x, float *out_y, float *out_z);

void engine_math_cross_product_float_v(float value, float in_x, float in_y, float *out_x, float *out_y);
void engine_math_cross_product_v_float(float in_x, float in_y, float value, float *out_x, float *out_y);


float engine_math_vector_length(float vx, float vy);
float engine_math_3d_vector_length(float vx, float vy, float vz);

void engine_math_normalize(float *vx, float *vy);
void engine_math_3d_normalize(float *vx, float *vy, float *vz);

float engine_math_vector_length_sqr(float x, float y);

float engine_math_clamp(float value, float min, float max);
float engine_math_map(float value, float in_min, float in_max, float out_min, float out_max);
float engine_math_map_clamp_out(float value, float in_min, float in_max, float out_min, float out_max);

void engine_math_swap(float *val0, float *val1);

bool engine_math_compare_floats(float value0, float value1);

float engine_math_angle_between(float px0, float py0, float px1, float py1);
float engine_math_distance_between_sqrd(float px0, float py0, float px1, float py1);
float engine_math_distance_between(float px0, float py0, float px1, float py1);
bool engine_math_int32_between(int32_t value, int32_t min, int32_t max);

uint32_t engine_math_2d_to_1d_index(uint32_t x, uint32_t y, uint32_t width);

void engine_math_2d_midpoint(float x0, float y0, float x1, float y1, float *mx, float *my);

bool engine_math_2d_do_segments_intersect(float line_0_start_x, float line_0_start_y, float line_0_end_x, float line_0_end_y, 
                                          float line_1_start_x, float line_1_start_y, float line_1_end_x, float line_1_end_y,
                                          float *intersect_x, float *intersect_y);

// Rotate a point '(px, py)' about another center point '(cx, cy)'
void engine_math_rotate_point(float *px, float *py, float cx, float cy, float angle_radians);

// Scales a point from a center position
void engine_math_scale_point(float *px, float *py, float cx, float cy, float scale);

float engine_math_fast_sin(float angle_radians);

#endif  // ENGINE_MATH_H