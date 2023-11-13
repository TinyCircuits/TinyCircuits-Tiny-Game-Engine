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


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y, vector3_class_obj_t *camera_position, rectangle_class_obj_t *camera_viewport){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(camera_position == NULL || camera_viewport == NULL){
        if(is_xy_inside_screen(x, y)){
            screen_buffer[y*SCREEN_WIDTH + x] = color;
        }else{
            ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
        }
    }else{
        int32_t vx = (int32_t)mp_obj_get_float(camera_viewport->x);
        int32_t vy = (int32_t)mp_obj_get_float(camera_viewport->y);
        uint8_t vw = (uint8_t)mp_obj_get_float(camera_viewport->width);
        uint8_t vh = (uint8_t)mp_obj_get_float(camera_viewport->height);

        int32_t result_x = ((int32_t)vx) + (x - (int32_t)camera_position->x);
        int32_t result_y = ((int32_t)vy) + (y - (int32_t)camera_position->y);

        if(is_xy_inside_viewport(result_x, result_y, vx, vy, vw, vh)){
            screen_buffer[result_y*SCREEN_WIDTH + result_x] = color;
        }else{
            ENGINE_WARNING_PRINTF("Tried to draw pixel outside of viewport bounds, clipped");
        }
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

#include "tan_sin_tab.h"

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
        a = (negative) ? tan_sin_tab[idx] : -tan_sin_tab[idx];
        b = (negative) ? -tan_sin_tab[idx+1] : tan_sin_tab[idx+1];
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
