#include "engine_math.h"
#include "trig_tables.h"
#include "math.h"
#include "debug/debug_print.h"

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


void engine_math_sin_tan(float angle_radians, int32_t *sin_output, int32_t *tan_output, bool *flip){
    // Not sure what's going on here
    int16_t theta_index = angle_radians * 1024 / (2.0f*PI);

    // Remainder after division by 1024: https://stackoverflow.com/a/11077172
    theta_index = theta_index & 1023;

    *flip = 0;
    if(theta_index > 512){
        theta_index -= 1024;
    } 
    if(theta_index > 256){
        *flip = 1;
        theta_index -= 512;
    } else if(theta_index < -256){
        *flip = 1;
        theta_index += 512;
    }

    if(theta_index != 256){
        if(theta_index < 0){
            theta_index = -theta_index;
            *sin_output = tan_table[theta_index];
            *tan_output = -sin_table[theta_index];
        }else{
            *sin_output = -tan_table[theta_index];
            *tan_output = sin_table[theta_index];
        }
    }else{
        if(theta_index < 0){
            *sin_output = 65536;
            *tan_output = -65536;
        }else{
            *sin_output = -65536;
            *tan_output = 65536;
        }
    }
}