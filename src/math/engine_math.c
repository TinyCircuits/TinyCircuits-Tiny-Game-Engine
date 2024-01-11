#include "engine_math.h"
#include "trig_tables.h"
#include "math.h"

// https://stackoverflow.com/a/22491252
void engine_math_rotate_point(float *px, float *py, float cx, float cy, float angle_radians){
    float x_centered = *px - cx;
    float y_centered = *py - cy;

    // Store these so it doesn't have to do it 2x
    float cos_angle = cosf(angle_radians);
    float sin_angle = sinf(angle_radians);

    float x_center_rotated = x_centered * cos_angle - y_centered * sin_angle;
    float y_center_rotated = x_centered * sin_angle + y_centered * cos_angle;

    *px = x_center_rotated + cx;
    *py = y_center_rotated + cy;
}


void engine_math_sin_tan(float angle_radians, int32_t *sin_output, int32_t *tan_output){
    // Get theta inside (-pi/2, pi/2)
    int16_t theta = angle_radians * 1024 / (2*PI);
    theta &= 0x3FF;
    if(theta > 0x200) theta -= 0x400;
    if(theta > 0x100){
        theta -= 0x200;
    } else if(theta < -0x100){
        theta += 0x200;
    }

    int negative = 0;
    if(theta < 0){
        negative = 1;
        theta = -theta;
    }

    int idx = (theta << 1);
    if(idx != 512){
        sin_output = (negative) ? tan_table[idx] : -tan_table[idx];
        tan_output = (negative) ? -sin_table[idx] : sin_table[idx];
    }else{
        sin_output = (negative) ? 65536 : -65536;
        tan_output = (negative) ? -65536 : 65536;
    }
}