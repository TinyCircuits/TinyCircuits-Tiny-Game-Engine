#include "engine_math.h"
#include "math.h"
#include "debug/debug_print.h"


float engine_math_dot_product(float x0, float y0, float x1, float y1){
    return x0*x1 + y0*y1;
}

// https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/IEMath.h#L261-L274
float engine_math_cross_product_v_v(float x0, float y0, float x1, float y1){
    return x0*y1 - y0*x1;
}

// http://labman.phys.utk.edu/3D%20Physics/crossproduct/crossproduct.html#:~:text=of%20the%20vector-,C%20%3D%20A%C3%97%20B,-and%20its%20magnitude
void engine_math_3d_cross_product_v_v(float ax, float ay, float az, float bx, float by, float bz, float *out_x, float *out_y, float *out_z){
    *out_x = ay*bz - az*by;
    *out_y = az*bx - ax*bz;
    *out_z = ax*by - ay*bx;
}

void engine_math_cross_product_float_v(float value, float in_x, float in_y, float *out_x, float *out_y){
    *out_x = -value * in_y;
    *out_y =  value * in_x;
}

void engine_math_cross_product_v_float(float in_x, float in_y, float value, float *out_x, float *out_y){
    *out_x =  value * in_y;
    *out_y = -value * in_x;
}


float engine_math_vector_length(float vx, float vy){
    return sqrtf((vx) * (vx) + (vy) * (vy));
}


float engine_math_3d_vector_length(float vx, float vy, float vz){
    return sqrtf((vx) * (vx) + (vy) * (vy) + (vz) * (vz));
}


// https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/IEMath.h#L144-L155
void engine_math_normalize(float *vx, float *vy){
    float length = engine_math_vector_length(*vx, *vy);

    if(length == 0.0f){
        // The length is zero so all components are zero, no use scaling them by anything.
        return;
    }

    const float factor = 1.0f / length;
    *vx *= factor;
    *vy *= factor;
}


// https://stackoverflow.com/a/19301193
void engine_math_3d_normalize(float *vx, float *vy, float *vz){
    float length = engine_math_3d_vector_length(*vx, *vy, *vz);

    if(length == 0.0f){
        // The length is zero so all components are zero, no use scaling them by anything.
        return;
    }

    const float factor = 1.0f / length;
    *vx *= factor;
    *vy *= factor;
    *vz *= factor;
}


float engine_math_vector_length_sqr(float x, float y){
    return x*x + y*y;
}


// https://stackoverflow.com/a/16659263
float engine_math_clamp(float value, float min, float max){
    const float t = value < min ? min : value;
    return t > max ? max : t;
}


float engine_math_map(float value, float in_min, float in_max, float out_min, float out_max){
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


float engine_math_map_clamp(float value, float in_min, float in_max, float out_min, float out_max){
    return value <= in_min
        ? out_min
        : value >= in_max
        ? out_max
        : engine_math_map(value, in_min, in_max, out_min, out_max);
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
    return atan2f(py1 - py0, px1 - px0);
}


float engine_math_distance_between_sqrd(float px0, float py0, float px1, float py1){
    float dx = px1 - px0;
    float dy = py1 - py0;
    return (dx*dx) + (dy*dy);
}


float engine_math_distance_between(float px0, float py0, float px1, float py1){
    return sqrtf(engine_math_distance_between_sqrd(px0, py0, px1, py1));
}

bool engine_math_int32_between(int32_t value, int32_t min, int32_t max){
    return (value >= min && value <= max);
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
// https://stackoverflow.com/a/49135089
void engine_math_scale_point(float *px, float *py, float cx, float cy, float scale){
    *px = scale * (*px-cx) + cx;
    *py = scale * (*py-cy) + cy;
}


int16_t sin_table[512] = {
0,
402,
805,
1208,
1610,
2013,
2415,
2816,
3217,
3618,
4018,
4418,
4817,
5215,
5612,
6009,
6404,
6799,
7193,
7585,
7976,
8367,
8756,
9143,
9529,
9914,
10297,
10679,
11059,
11438,
11814,
12189,
12562,
12933,
13302,
13670,
14035,
14398,
14759,
15117,
15474,
15827,
16179,
16528,
16875,
17219,
17560,
17899,
18235,
18569,
18899,
19227,
19552,
19873,
20192,
20508,
20821,
21130,
21437,
21740,
22039,
22336,
22629,
22919,
23205,
23488,
23767,
24042,
24314,
24582,
24847,
25108,
25365,
25618,
25867,
26112,
26354,
26591,
26825,
27054,
27279,
27500,
27717,
27930,
28139,
28343,
28543,
28739,
28930,
29117,
29300,
29478,
29651,
29821,
29985,
30145,
30301,
30452,
30599,
30740,
30877,
31010,
31138,
31261,
31379,
31493,
31602,
31706,
31805,
31900,
31989,
32074,
32154,
32229,
32299,
32365,
32425,
32481,
32531,
32577,
32618,
32654,
32685,
32711,
32732,
32748,
32759,
32765,
32766,
32763,
32754,
32740,
32722,
32698,
32670,
32636,
32598,
32555,
32507,
32454,
32395,
32333,
32265,
32192,
32115,
32032,
31945,
31853,
31756,
31654,
31548,
31437,
31321,
31200,
31074,
30944,
30810,
30670,
30526,
30377,
30224,
30066,
29904,
29737,
29565,
29389,
29209,
29024,
28835,
28641,
28443,
28241,
28035,
27824,
27609,
27390,
27167,
26940,
26708,
26473,
26234,
25990,
25743,
25492,
25237,
24978,
24715,
24449,
24179,
23905,
23628,
23347,
23062,
22774,
22483,
22188,
21890,
21589,
21284,
20976,
20665,
20351,
20033,
19713,
19390,
19063,
18734,
18402,
18068,
17730,
17390,
17047,
16702,
16354,
16004,
15651,
15296,
14938,
14578,
14217,
13853,
13486,
13118,
12748,
12376,
12002,
11626,
11249,
10869,
10488,
10106,
9722,
9336,
8950,
8561,
8172,
7781,
7389,
6996,
6602,
6207,
5811,
5414,
5016,
4617,
4218,
3818,
3418,
3017,
2616,
2214,
1812,
1409,
1007,
604,
201,
-201,
-604,
-1007,
-1409,
-1812,
-2214,
-2616,
-3017,
-3418,
-3818,
-4218,
-4617,
-5016,
-5414,
-5811,
-6207,
-6602,
-6996,
-7389,
-7781,
-8172,
-8561,
-8950,
-9336,
-9722,
-10106,
-10488,
-10869,
-11249,
-11626,
-12002,
-12376,
-12748,
-13118,
-13486,
-13853,
-14217,
-14578,
-14938,
-15296,
-15651,
-16004,
-16354,
-16702,
-17047,
-17390,
-17730,
-18068,
-18402,
-18734,
-19063,
-19390,
-19713,
-20033,
-20351,
-20665,
-20976,
-21284,
-21589,
-21890,
-22188,
-22483,
-22774,
-23062,
-23347,
-23628,
-23905,
-24179,
-24449,
-24715,
-24978,
-25237,
-25492,
-25743,
-25990,
-26234,
-26473,
-26708,
-26940,
-27167,
-27390,
-27609,
-27824,
-28035,
-28241,
-28443,
-28641,
-28835,
-29024,
-29209,
-29389,
-29565,
-29737,
-29904,
-30066,
-30224,
-30377,
-30526,
-30670,
-30810,
-30944,
-31074,
-31200,
-31321,
-31437,
-31548,
-31654,
-31756,
-31853,
-31945,
-32032,
-32115,
-32192,
-32265,
-32333,
-32395,
-32454,
-32507,
-32555,
-32598,
-32636,
-32670,
-32698,
-32722,
-32740,
-32754,
-32763,
-32766,
-32765,
-32759,
-32748,
-32732,
-32711,
-32685,
-32654,
-32618,
-32577,
-32531,
-32481,
-32425,
-32365,
-32299,
-32229,
-32154,
-32074,
-31989,
-31900,
-31805,
-31706,
-31602,
-31493,
-31379,
-31261,
-31138,
-31010,
-30877,
-30740,
-30599,
-30452,
-30301,
-30145,
-29985,
-29821,
-29651,
-29478,
-29300,
-29117,
-28930,
-28739,
-28543,
-28343,
-28139,
-27930,
-27717,
-27500,
-27279,
-27054,
-26825,
-26591,
-26354,
-26112,
-25867,
-25618,
-25365,
-25108,
-24847,
-24582,
-24314,
-24042,
-23767,
-23488,
-23205,
-22919,
-22629,
-22336,
-22039,
-21740,
-21437,
-21130,
-20821,
-20508,
-20192,
-19873,
-19552,
-19227,
-18899,
-18569,
-18235,
-17899,
-17560,
-17219,
-16875,
-16528,
-16179,
-15827,
-15474,
-15117,
-14759,
-14398,
-14035,
-13670,
-13302,
-12933,
-12562,
-12189,
-11814,
-11438,
-11059,
-10679,
-10297,
-9914,
-9529,
-9143,
-8756,
-8367,
-7976,
-7585,
-7193,
-6799,
-6404,
-6009,
-5612,
-5215,
-4817,
-4418,
-4018,
-3618,
-3217,
-2816,
-2415,
-2013,
-1610,
-1208,
-805,
-402,
0
};


float engine_math_fast_sin(float angle_radians){
    angle_radians = fmodf(angle_radians, TWICE_PI);
    angle_radians *= 512 / TWICE_PI;
    return (float)((float)sin_table[(uint16_t)angle_radians] / (float)INT16_MAX);
}