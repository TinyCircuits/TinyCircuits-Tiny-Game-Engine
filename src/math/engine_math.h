#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include <stdint.h>

#define PI      3.14159265358979323846f
#define RAD2DEG (180.0f/PI)
#define DEG2RAD (PI/180.0f)


// Rotate a point '(px, py)' about another center point '(cx, cy)'
void engine_math_rotate_point(float *px, float *py, float cx, float cy, float angle_radians);
void engine_math_sin_tan(float angle_radians, int32_t *sin_output, int32_t *tan_output);

#endif  // ENGINE_MATH_H