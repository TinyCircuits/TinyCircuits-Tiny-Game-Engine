#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"

#include <string.h>


inline bool is_xy_inside_viewport(int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh){
    if(x >= vx && y >= vy && x < vx+vw && y < vy+vh){
        return true;
    }

    return false;
}


inline bool is_xy_inside_screen(int32_t x, int32_t y){
    if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT){
        return true;
    }

    return false;
}


void engine_draw_pixel_viewport(uint16_t color, int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh, int32_t px, int32_t py){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    int32_t result_x = vx + (x - px);
    int32_t result_y = vy + (y - py);

    if(is_xy_inside_viewport(result_x, result_y, vx, vy, vw, vh)){
        screen_buffer[result_y*SCREEN_WIDTH + result_x] = color;
    }else{
        ENGINE_WARNING_PRINTF("Tried to draw pixel outside of viewport bounds, clipped");
    }
}


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(is_xy_inside_screen(x, y)){
        screen_buffer[y*SCREEN_WIDTH + x] = color;
    }else{
        ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
    }
}


void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer){
    memset(screen_buffer, color, SCREEN_BUFFER_SIZE_BYTES);
}

#include "hardware/interp.h"

void init_interp(int t_xs_log2)
{
  interp_config cfg = interp_default_config();
  interp_config_set_signed(&cfg, false);
  interp_config_set_add_raw(&cfg, true);
  interp_config_set_shift(&cfg, 16-t_xs_log2);
  interp_config_set_mask(&cfg, t_xs_log2, 31);
  interp_set_config(interp0, 0, &cfg);
  interp_config_set_shift(&cfg, 16);
  interp_config_set_mask(&cfg, 0, 31);
  interp_set_config(interp0, 1, &cfg);
}

// 16-bit tangent, sine table from 0..pi/2.
const static uint16_t sin_tan_tab[512] = {
    0, 0,
    201, 402,
    402, 804,
    603, 1206,
    804, 1608,
    1005, 2010,
    1207, 2412,
    1408, 2814,
    1609, 3216,
    1810, 3617,
    2011, 4019,
    2213, 4420,
    2414, 4821,
    2615, 5222,
    2817, 5623,
    3018, 6023,
    3220, 6424,
    3421, 6824,
    3623, 7224,
    3825, 7623,
    4026, 8022,
    4228, 8421,
    4430, 8820,
    4632, 9218,
    4834, 9616,
    5036, 10014,
    5239, 10411,
    5441, 10808,
    5644, 11204,
    5846, 11600,
    6049, 11996,
    6252, 12391,
    6455, 12785,
    6658, 13180,
    6861, 13573,
    7064, 13966,
    7268, 14359,
    7471, 14751,
    7675, 15143,
    7879, 15534,
    8083, 15924,
    8287, 16314,
    8492, 16703,
    8696, 17091,
    8901, 17479,
    9106, 17867,
    9311, 18253,
    9516, 18639,
    9721, 19024,
    9927, 19409,
    10133, 19792,
    10339, 20175,
    10545, 20557,
    10751, 20939,
    10958, 21320,
    11165, 21699,
    11372, 22078,
    11579, 22457,
    11786, 22834,
    11994, 23210,
    12202, 23586,
    12410, 23961,
    12618, 24335,
    12827, 24708,
    13036, 25080,
    13245, 25451,
    13454, 25821,
    13664, 26190,
    13874, 26558,
    14084, 26925,
    14295, 27291,
    14506, 27656,
    14717, 28020,
    14928, 28383,
    15140, 28745,
    15352, 29106,
    15564, 29466,
    15776, 29824,
    15989, 30182,
    16202, 30538,
    16416, 30893,
    16630, 31248,
    16844, 31600,
    17058, 31952,
    17273, 32303,
    17489, 32652,
    17704, 33000,
    17920, 33347,
    18136, 33692,
    18353, 34037,
    18570, 34380,
    18787, 34721,
    19005, 35062,
    19223, 35401,
    19442, 35738,
    19661, 36075,
    19880, 36410,
    20100, 36744,
    20320, 37076,
    20541, 37407,
    20762, 37736,
    20983, 38064,
    21205, 38391,
    21427, 38716,
    21650, 39040,
    21873, 39362,
    22097, 39683,
    22321, 40002,
    22546, 40320,
    22771, 40636,
    22997, 40951,
    23223, 41264,
    23449, 41576,
    23676, 41886,
    23904, 42194,
    24132, 42501,
    24360, 42806,
    24590, 43110,
    24819, 43412,
    25049, 43713,
    25280, 44011,
    25511, 44308,
    25743, 44604,
    25975, 44898,
    26208, 45190,
    26442, 45480,
    26676, 45769,
    26911, 46056,
    27146, 46341,
    27382, 46624,
    27618, 46906,
    27855, 47186,
    28093, 47464,
    28331, 47741,
    28570, 48015,
    28810, 48288,
    29050, 48559,
    29291, 48828,
    29533, 49095,
    29775, 49361,
    30018, 49624,
    30261, 49886,
    30506, 50146,
    30751, 50404,
    30996, 50660,
    31243, 50914,
    31490, 51166,
    31738, 51417,
    31986, 51665,
    32236, 51911,
    32486, 52156,
    32736, 52398,
    32988, 52639,
    33240, 52878,
    33494, 53114,
    33748, 53349,
    34002, 53581,
    34258, 53812,
    34514, 54040,
    34772, 54267,
    35030, 54491,
    35289, 54714,
    35548, 54934,
    35809, 55152,
    36071, 55368,
    36333, 55582,
    36596, 55794,
    36861, 56004,
    37126, 56212,
    37392, 56418,
    37659, 56621,
    37927, 56823,
    38196, 57022,
    38465, 57219,
    38736, 57414,
    39008, 57607,
    39281, 57798,
    39555, 57986,
    39829, 58172,
    40105, 58356,
    40382, 58538,
    40660, 58718,
    40939, 58896,
    41219, 59071,
    41500, 59244,
    41782, 59415,
    42066, 59583,
    42350, 59750,
    42636, 59914,
    42923, 60075,
    43210, 60235,
    43500, 60392,
    43790, 60547,
    44081, 60700,
    44374, 60851,
    44668, 60999,
    44963, 61145,
    45259, 61288,
    45557, 61429,
    45856, 61568,
    46156, 61705,
    46457, 61839,
    46760, 61971,
    47064, 62101,
    47369, 62228,
    47676, 62353,
    47984, 62476,
    48294, 62596,
    48605, 62714,
    48917, 62830,
    49231, 62943,
    49546, 63054,
    49863, 63162,
    50181, 63268,
    50501, 63372,
    50822, 63473,
    51145, 63572,
    51469, 63668,
    51795, 63763,
    52122, 63854,
    52451, 63944,
    52782, 64031,
    53114, 64115,
    53448, 64197,
    53784, 64277,
    54121, 64354,
    54460, 64429,
    54801, 64501,
    55144, 64571,
    55488, 64639,
    55834, 64704,
    56182, 64766,
    56532, 64827,
    56883, 64884,
    57237, 64940,
    57592, 64993,
    57950, 65043,
    58309, 65091,
    58670, 65137,
    59033, 65180,
    59398, 65220,
    59766, 65259,
    60135, 65294,
    60506, 65328,
    60880, 65358,
    61255, 65387,
    61633, 65413,
    62013, 65436,
    62395, 65457,
    62780, 65476,
    63167, 65492,
    63556, 65505,
    63947, 65516,
    64341, 65525,
    64737, 65531,
    65135, 65535,
};

void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){

}

void engine_draw_blit_scale(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc) {
    int32_t width = 1u << width_log2;
    int32_t xe = (width * xsc) >> 16;
    int32_t ye = (height * ysc) >> 16;
    int32_t dtx = ((int64_t)width << 16) / xe;
    int32_t dty = ((int64_t)height << 16) / ye;
    int32_t ty = 0x8000;
    int32_t tx = 0x8000;
    if(xsc < 0) {
        xe = -xe;
        x -= xe;
    }
    if(ysc < 0) {
        ye = -ye;
        y -= ye;
        ty = (height << 16) - 0x8000;
    }
    //printf("dtx=%i, dty=%i\n\r", dtx, dty);
    //interp_set_config(interp0, 0, &cfg);
    int32_t fb_pos = y * SCREEN_WIDTH + x;
    init_interp(width_log2);
    uint16_t *screen_buffer = engine_get_active_screen_buffer();
    for(int cy = 0; cy < ye; cy++) {
        tx = (xsc < 0) ? ((width << 16) - 0x8000) : 0x8000;
        interp0->accum[1] = tx;
        interp0->base[1] = dtx;
        interp0->accum[0] = ty;
        interp0->base[0] = 0;
        for(int cx = 0; cx < xe; cx++) {

            // fb[fb_pos + cx] = tex[((ty >> 16) << t_xs_log2) + (tx >> 16)];
            // tx += dtx;

            screen_buffer[fb_pos + cx] = pixels[interp_pop_full_result(interp0)];
        }
        fb_pos += SCREEN_WIDTH;
        ty += dty;
    }
}

void engine_draw_blit_scale_trishear(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int flip) {
    int32_t width = 1u << width_log2;
    int32_t xe = (width * xsc) >> 16;
    int32_t ye = (height * ysc) >> 16;
    int32_t dtx = ((int64_t)width << 16) / xe;
    int32_t dty = ((int64_t)height << 16) / ye;
    int32_t ty = 0;
    int32_t tx = 0;

    if(xsc < 0) {
        xe = -xe;
        x -= xe;
        // dtx = -dtx;
    }
    if(ysc < 0) {
        ye = -ye;
        y -= ye;
        ty = (height << 16) - 0x10000;
        // dty = -dty;
    }

    int32_t fb_pos = y * SCREEN_WIDTH + x;
    int32_t x_start = ((xsc < 0)) ? ((width << 16) - 0x10000) : 0;

    int32_t xshift = 0;
    int32_t yshift = 0;
    int32_t xshift2 = 0;
    init_interp(width_log2);
    uint16_t *screen_buffer = engine_get_active_screen_buffer();
    for(int cy = 0; cy < ye; cy++) {
        yshift = (xshift >> 16) * ysr;
        tx = x_start;
        fb_pos += (xshift >> 16);
        interp0->accum[1] = tx;
        interp0->base[1] = dtx;
        interp0->accum[0] = ty;
        interp0->base[0] = 0;
        if(flip) for(int cx = 0; cx < xe; cx++) {
            //printf("tx, ty are %f, %f\n\r", tx/65536.f, ty/65536.f);
            xshift2 = ((cy + (yshift >> 16)) * xsr2);

            //fb[fb_pos + (cx) + (yshift >> 16) * f_xs + (xshift2 >> 16)] = tex[t_xs * t_ys - 1 - ((ty >> 16) * t_xs + (tx >> 16))];
            screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[width * height - 1 - interp_pop_full_result(interp0)];

            //tx += dtx;
            yshift += ysr;
        }
        else for(int cx = 0; cx < xe; cx++) {
            //printf("tx, ty are %f, %f\n\r", tx/65536.f, ty/65536.f);
            xshift2 = ((cy + (yshift >> 16)) * xsr2);

            //fb[fb_pos + (cx) + (yshift >> 16) * f_xs + (xshift2 >> 16)] = tex[(ty >> 16) * t_xs + (tx >> 16)];
            screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[interp_pop_full_result(interp0)];

            //tx += dtx;
            yshift += ysr;
        }
        fb_pos -= (xshift >> 16);
        fb_pos += SCREEN_WIDTH;
        xshift += xsr;
        ty += dty;
    }
}

void engine_draw_blit_scale_rotate(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int16_t theta) {
    int flip = 0;
    int32_t width = 1u << width_log2;
    // Step 1: Get theta inside (-pi/2, pi/2) and flip if we need to
    theta &= 0x3FF;
    if(theta > 0x200) theta -= 0x400;
    if(theta > 0x100) {
        flip = 1;
        theta -= 0x200;
    } else if(theta < -0x100) {
        flip = 1;
        theta += 0x200;
    }

    int negative = 0;
    if(theta < 0) {
        negative = 1;
        theta = -theta;
    }

    //mp_printf(&mp_plat_print, "Converted theta to %f\n\r", theta * 3.14159265 / 256.f);

    int idx = (theta << 1);
    //printf("Table index is %i\n\r", idx);
    int32_t a, b; // tan(theta*0.5), sin(theta)
    if(idx != 512) {
        a = (negative) ? sin_tan_tab[idx] : -sin_tan_tab[idx];
        b = (negative) ? -sin_tan_tab[idx+1] : sin_tan_tab[idx+1];
    } else {
        a = (negative) ? 65536 : -65536;
        b = (negative) ? -65536 : 65536;
    }
    int32_t c = (((int64_t)a*b) >> 16) + 0x10000;
    //printf("tan(0.5t)=%f, sin(t)=%f\n\r", a/65536.f, b/65536.f);
    //printf("Flip: %i\n\r", flip);
    // Step 3: Rotate center w.r.t. pivot so we can rotate about the center instead
    int32_t xe = ((int64_t)width * xsc) >> 16;
    int32_t ye = ((int64_t)height * ysc) >> 16;
    // if(xe < 0) xe = -xe;
    // if(ye < 0) ye = -ye;
    if(xsc < 0) xe = -xe;
    if(ysc < 0) ye = -ye;
    int cx = ((int64_t)(xe/2) * c - (int64_t)(ye/2) * b) >> 16;
    int cy = ((int64_t)(ye/2) * c + (int64_t)(xe/2) * b) >> 16;
    if(xsc < 0) cx -= xe;
    if(ysc < 0) cy -= ye;
    //Step 4: Triple shear (a, b, a);
    //blit_scale_trishear_pow2_tex_internal(fb, f_xs, tex, t_xs_log2, t_ys, x - cx, y - cy, xsc, ysc, a, b, a, flip);
    engine_draw_blit_scale_trishear(pixels, x - cx, y - cy, width_log2, height, xsc, ysc, a, b, a, flip);
}
