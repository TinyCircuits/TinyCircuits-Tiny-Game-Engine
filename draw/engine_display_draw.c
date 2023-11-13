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

void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){

}

void engine_draw_blit_scale(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc) {

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
