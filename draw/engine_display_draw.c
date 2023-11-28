#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"
#include "tan_sin_tab.h"
#include <string.h>

#ifndef __unix__
    #include "hardware/interp.h"
#endif



void engine_draw_fill(uint16_t color, uint16_t *screen_buffer){
    memset(screen_buffer, color, SCREEN_BUFFER_SIZE_BYTES);
}


bool engine_draw_pixel(uint16_t color, uint8_t x, uint8_t y){
    if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT){
        uint16_t *screen_buffer = engine_get_active_screen_buffer();
        uint16_t index = y * SCREEN_WIDTH + x;

        screen_buffer[index] = color;

        return true;
    }else{
        return false;
    }
}


void engine_draw_vertical_line(uint16_t color, uint8_t x_column, uint8_t y_start, uint8_t y_end){

}


void engine_draw_horizontal_line(uint16_t color, uint8_t y_row, uint8_t x_start, uint8_t x_end){

}


void engine_draw_sloped_line(uint16_t color, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end){

}


void engine_draw_line(uint16_t color, uint8_t x_start, uint8_t y_start, uint8_t x_end, uint8_t y_end){
    
}


void engine_draw_outline_rectangle(uint16_t color, uint8_t x_top_left, uint8_t y_top_left, uint8_t width, uint8_t height){

}


void engine_draw_filled_rectangle(uint16_t color, uint8_t x_top_left, uint8_t y_top_left, uint8_t width, uint8_t height){

}


void engine_draw_aligned_blit(uint16_t *buffer, uint8_t x_top_left, uint8_t y_top_left, uint8_t width, uint8_t height, uint16_t key, bool mirror_x, bool mirror_y){

}


void engine_draw_aligned_blit_with_mask(uint16_t *buffer, uint8_t x_top_left, uint8_t y_top_left, uint8_t width, uint8_t height, uint16_t mask_key, bool mirror_x, bool mirror_y, uint16_t *mask_buffer){

}



inline bool is_xy_inside_viewport(int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh){
    if(x >= vx && y >= vy && x < vx+vw && y < vy+vh){
        return true;
    }

    return false;
}


// inline bool is_xy_inside_screen(int32_t x, int32_t y){
//     if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT){
//         return true;
//     }

//     return false;
// }


// void engine_draw_pixel_viewport(uint16_t color, int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh, int32_t px, int32_t py){
//     uint16_t *screen_buffer = engine_get_active_screen_buffer();

//     int32_t result_x = vx + (x - px);
//     int32_t result_y = vy + (y - py);

//     if(is_xy_inside_viewport(result_x, result_y, vx, vy, vw, vh)){
//         screen_buffer[result_y*SCREEN_WIDTH + result_x] = color;
//     }else{
//         ENGINE_WARNING_PRINTF("Tried to draw pixel outside of viewport bounds, clipped");
//     }
// }


// void engine_draw_pixel_unsafe(uint16_t color, int32_t x, int32_t y){
//     uint16_t *screen_buffer = engine_get_active_screen_buffer();
//     screen_buffer[y*SCREEN_WIDTH + x] = color;
// }


// void engine_draw_pixel(uint16_t color, int32_t x, int32_t y){
//     uint16_t *screen_buffer = engine_get_active_screen_buffer();

//     if(is_xy_inside_screen(x, y)){
//         screen_buffer[y*SCREEN_WIDTH + x] = color;
//     }else{
//         ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
//     }
// }


// void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer){
//     memset(screen_buffer, color, SCREEN_BUFFER_SIZE_BYTES);
// }


// #ifndef __unix__
//     void init_interp(int t_xs_log2){
//         interp_config cfg = interp_default_config();
//         interp_config_set_signed(&cfg, false);
//         interp_config_set_add_raw(&cfg, true);
//         interp_config_set_shift(&cfg, 16-t_xs_log2);
//         interp_config_set_mask(&cfg, t_xs_log2, 31);
//         interp_set_config(interp0, 0, &cfg);
//         interp_config_set_shift(&cfg, 16);
//         interp_config_set_mask(&cfg, 0, 31);
//         interp_set_config(interp0, 1, &cfg);
//     }
// #endif

// void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){

// }


// void engine_draw_blit_scale(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc){
//     #ifndef __unix__
//         init_interp(width_log2);
//     #endif

//     int32_t width = 1u << width_log2;
//     int32_t xe = (width * xsc) >> 16;
//     int32_t ye = (height * ysc) >> 16;
//     int32_t dtx = ((int64_t)width << 16) / xe;
//     int32_t dty = ((int64_t)height << 16) / ye;
//     int32_t ty = 0x8000;
//     int32_t tx = 0x8000;

//     if(xsc < 0){
//         xe = -xe;
//         x -= xe;
//     }
//     if(ysc < 0){
//         ye = -ye;
//         y -= ye;
//         ty = (height << 16) - 0x8000;
//     }

//     int32_t fb_pos = y * SCREEN_WIDTH + x;
//     uint16_t *screen_buffer = engine_get_active_screen_buffer();

//     for(int cy = 0; cy < ye; cy++){
//         tx = (xsc < 0) ? ((width << 16) - 0x8000) : 0x8000;

//         #ifndef __unix__
//             interp0->accum[1] = tx;
//             interp0->base[1] = dtx;
//             interp0->accum[0] = ty;
//             interp0->base[0] = 0;
//         #endif

//         for(int cx = 0; cx < xe; cx++){
//             #ifndef __unix__
//                 screen_buffer[fb_pos + cx] = pixels[interp_pop_full_result(interp0)];
//             #else
//                 screen_buffer[fb_pos + cx] = pixels[((ty >> 16) << width_log2) + (tx >> 16)];
//                 tx += dtx;
//             #endif
//         }

//         fb_pos += SCREEN_WIDTH;
//         ty += dty;
//     }
// }


// void engine_draw_blit_scale_trishear(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int flip){
//     #ifndef __unix__
//         init_interp(width_log2);
//     #endif

//     int32_t width = 1u << width_log2;
//     int32_t xe = (width * xsc) >> 16;
//     int32_t ye = (height * ysc) >> 16;
//     int32_t dtx = ((int64_t)width << 16) / xe;
//     int32_t dty = ((int64_t)height << 16) / ye;
//     int32_t ty = 0;
//     int32_t tx = 0;

//     if(xsc < 0){
//         xe = -xe;
//         x -= xe;
//     }
//     if(ysc < 0){
//         ye = -ye;
//         y -= ye;
//         ty = (height << 16) - 0x10000;
//     }

//     int32_t fb_pos = y * SCREEN_WIDTH + x;
//     int32_t x_start = ((xsc < 0)) ? ((width << 16) - 0x10000) : 0;

//     int32_t xshift = 0;
//     int32_t yshift = 0;
//     int32_t xshift2 = 0;
//     uint16_t *screen_buffer = engine_get_active_screen_buffer();

//     for(int cy = 0; cy < ye; cy++){
//         yshift = (xshift >> 16) * ysr;
//         tx = x_start;
//         fb_pos += (xshift >> 16);

//         #ifndef __unix__
//             interp0->accum[1] = tx;
//             interp0->base[1] = dtx;
//             interp0->accum[0] = ty;
//             interp0->base[0] = 0;
//         #endif

//         if(flip) for(int cx = 0; cx < xe; cx++){
//             xshift2 = ((cy + (yshift >> 16)) * xsr2);

//             #ifndef __unix__
//                 screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[width * height - 1 - interp_pop_full_result(interp0)];
//             #else
//                 screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[width * height - 1 - ((ty >> 16) * width + (tx >> 16))];
//                 tx += dtx;
//             #endif

//             yshift += ysr;
//         }else for(int cx = 0; cx < xe; cx++){
//             xshift2 = ((cy + (yshift >> 16)) * xsr2);

//             #ifndef __unix__
//                 screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[interp_pop_full_result(interp0)];
//             #else
//                 screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[(ty >> 16) * width + (tx >> 16)];
//                 tx += dtx;
//             #endif

//             yshift += ysr;
//         }

//         fb_pos -= (xshift >> 16);
//         fb_pos += SCREEN_WIDTH;
//         xshift += xsr;
//         ty += dty;
//     }
// }

void engine_draw_fillrect_scale_trishear_viewport(uint16_t color, int32_t x, int32_t y, uint16_t width, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh){

    int32_t xe = (width * xsc) >> 16;
    int32_t ye = (height * ysc) >> 16;
    int32_t fb_pos = y * SCREEN_WIDTH;

    int32_t xshift = 0;
    int64_t xshift2 = 0;
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(xsc < 0) {
        xe = -xe;
        x -= xe;
    }
    if(ysc < 0) {
        ye = -ye;
        y -= ye;
    }

    for(int cy = y; cy < y + ye; cy++){
        fb_pos += (xshift >> 16);
        int32_t yp = (cy << 16) + (xshift >> 16) * ysr;
        for(int cx = x; cx < x + xe; cx++) {
            xshift2 = (((yp >> 16) - y) * xsr2);
            int32_t xp = cx + (xshift >> 16) + (xshift2 >> 16);
            if(is_xy_inside_viewport(xp, yp >> 16, vx, vy, vw, vw)) screen_buffer[fb_pos + (cx) + ((yp >> 16) - cy) * SCREEN_WIDTH + (xshift2 >> 16)] = color;
            yp += ysr;
        }

        fb_pos -= (xshift >> 16);
        fb_pos += SCREEN_WIDTH;
        xshift += xsr;
    }
}

// void engine_draw_blit_scale_rotate(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int16_t theta){
//     int flip = 0;
//     int32_t width = 1u << width_log2;
//     // Step 1: Get theta inside (-pi/2, pi/2) and flip if we need to
//     theta &= 0x3FF;
//     if(theta > 0x200) theta -= 0x400;
//     if(theta > 0x100){
//         flip = 1;
//         theta -= 0x200;
//     } else if(theta < -0x100){
//         flip = 1;
//         theta += 0x200;
//     }

//     int negative = 0;
//     if(theta < 0){
//         negative = 1;
//         theta = -theta;
//     }

//     int idx = (theta << 1);
//     int32_t a, b;
//     if(idx != 512){
//         a = (negative) ? tan_sin_tab[idx] : -tan_sin_tab[idx];
//         b = (negative) ? -tan_sin_tab[idx+1] : tan_sin_tab[idx+1];
//     }else{
//         a = (negative) ? 65536 : -65536;
//         b = (negative) ? -65536 : 65536;
//     }
//     int32_t c = (((int64_t)a*b) >> 16) + 0x10000;


//     // Step 3: Rotate center w.r.t. pivot so we can rotate about the center instead
//     int32_t xe = ((int64_t)width * xsc) >> 16;
//     int32_t ye = ((int64_t)height * ysc) >> 16;
//     if(xsc < 0) xe = -xe;
//     if(ysc < 0) ye = -ye;
//     int cx = ((int64_t)(xe/2) * c - (int64_t)(ye/2) * b) >> 16;
//     int cy = ((int64_t)(ye/2) * c + (int64_t)(xe/2) * b) >> 16;
//     if(xsc < 0) cx -= xe;
//     if(ysc < 0) cy -= ye;
//     //Step 4: Triple shear (a, b, a);
//     //blit_scale_trishear_pow2_tex_internal(fb, f_xs, tex, t_xs_log2, t_ys, x - cx, y - cy, xsc, ysc, a, b, a, flip);
//     engine_draw_blit_scale_trishear(pixels, x - cx, y - cy, width_log2, height, xsc, ysc, a, b, a, flip);
// }


void engine_draw_fillrect_scale_rotate_viewport(uint16_t color, int32_t x, int32_t y, uint16_t width, uint16_t height, int32_t xsc, int32_t ysc, int16_t theta, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh){
    // Step 1: Get theta inside (-pi/2, pi/2) and flip if we need to
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
    int32_t a, b;
    if(idx != 512){
        a = (negative) ? tan_sin_tab[idx] : -tan_sin_tab[idx];
        b = (negative) ? -tan_sin_tab[idx+1] : tan_sin_tab[idx+1];
    }else{
        a = (negative) ? 65536 : -65536;
        b = (negative) ? -65536 : 65536;
    }
    int32_t c = (((int64_t)a*b) >> 16) + 0x10000;


    // Step 3: Rotate center w.r.t. pivot so we can rotate about the center instead
    int32_t xe = ((int64_t)width * xsc) >> 16;
    int32_t ye = ((int64_t)height * ysc) >> 16;
    if(xsc < 0) xe = -xe;
    if(ysc < 0) ye = -ye;
    int cx = ((int64_t)(xe/2) * c - (int64_t)(ye/2) * b) >> 16;
    int cy = ((int64_t)(ye/2) * c + (int64_t)(xe/2) * b) >> 16;
    if(xsc < 0) cx -= xe;
    if(ysc < 0) cy -= ye;
    //Step 4: Triple shear (a, b, a);
    //blit_scale_trishear_pow2_tex_internal(fb, f_xs, tex, t_xs_log2, t_ys, x - cx, y - cy, xsc, ysc, a, b, a, flip);
    engine_draw_fillrect_scale_trishear_viewport(color, x - cx, y - cy, width, height, xsc, ysc, a, b, a, vx, vy, vw, vh);
}
