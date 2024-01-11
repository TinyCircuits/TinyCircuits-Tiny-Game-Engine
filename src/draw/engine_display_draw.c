#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"
#include "tan_sin_tab.h"
#include <string.h>

#include "nodes/node_base.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "math/engine_math.h"

#ifndef __unix__
    #include "hardware/interp.h"
#endif


void engine_draw_fill(uint16_t color, uint16_t *screen_buffer){
    uint16_t *buf = screen_buffer;
    uint16_t count = SCREEN_BUFFER_SIZE_PIXELS;

    while(count--) *buf++ = color;
}


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y){
    if((x >= 0 && x < SCREEN_WIDTH) && (y >= 0 && y < SCREEN_HEIGHT)){
        uint16_t *screen_buffer = engine_get_active_screen_buffer();
        uint16_t index = y * SCREEN_WIDTH + x;

        screen_buffer[index] = color;
    }
}


// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
void engine_draw_line(uint16_t color, float x_start, float y_start, float x_end, float y_end, mp_obj_t camera_node_base_in){
    engine_node_base_t *camera_node_base = camera_node_base_in;
    vector2_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);

    // Viewport x and y are only used for location inside framebuffer (so not used here)
    float center_x = (float)camera_position->x + (float)camera_viewport->width/2.0f;
    float center_y = (float)camera_position->y + (float)camera_viewport->height/2.0f;

    // Rotate endpoints about camera
    engine_math_rotate_point(&x_start, &y_start, center_x, center_y, (float)camera_rotation->z * DEG2RAD);
    engine_math_rotate_point(&x_end, &y_end, center_x, center_y, (float)camera_rotation->z * DEG2RAD);

    // Distance difference between endpoints
    float dx = x_end - x_start;
    float dy = y_end - y_start;

    // See which axis requires most steps to draw complete line, store it
    float step_count = 0.0f;
    if(abs((int)dx) >= abs((int)dy)){
        step_count = abs((int)dx);
    }else{
        step_count = abs((int)dy);
    }

    // Calculate how much to increment each axis each step
    float slope_x = dx / step_count;
    float slope_y = dy / step_count;

    float line_x = x_start;
    float line_y = y_start;

    // Draw the line
    for(uint32_t step=0; step<step_count; step++){
        line_x = line_x + slope_x;
        line_y = line_y + slope_y;
        engine_draw_pixel(color, (int32_t)line_x, (int32_t)line_y);
    }
}


void engine_draw_outline_rectangle(uint16_t color, int32_t x_top_left, int32_t y_top_left, int32_t width, int32_t height){

}


void engine_draw_filled_rectangle(uint16_t color, int32_t x_top_left, int32_t y_top_left, int32_t width, int32_t height){

}


void engine_draw_blit(uint16_t *buffer, int32_t x_top_left, int32_t y_top_left, int32_t width, int32_t height, uint16_t key, bool mirror_x, bool mirror_y){

}



bool is_xy_inside_viewport(int32_t x, int32_t y, int32_t vx, int32_t vy, int32_t vw, int32_t vh){
    if(x >= vx && y >= vy && x < vx+vw && y < vy+vh){
        return true;
    }

    return false;
}



#ifndef __unix__
    void init_interp(int t_xs_log2){
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
#endif


void engine_draw_blit_scale(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc){
    #ifndef __unix__
        init_interp(width_log2);
    #endif

    int32_t width = 1u << width_log2;
    int32_t xe = (width * xsc) >> 16;
    int32_t ye = (height * ysc) >> 16;
    int32_t dtx = ((int64_t)width << 16) / xe;
    int32_t dty = ((int64_t)height << 16) / ye;
    int32_t ty = 0x8000;
    int32_t tx = 0x8000;

    if(xsc < 0){
        xe = -xe;
        x -= xe;
    }
    if(ysc < 0){
        ye = -ye;
        y -= ye;
        ty = (height << 16) - 0x8000;
    }

    int32_t fb_pos = y * SCREEN_WIDTH + x;
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    for(int cy = 0; cy < ye; cy++){
        tx = (xsc < 0) ? ((width << 16) - 0x8000) : 0x8000;

        #ifndef __unix__
            interp0->accum[1] = tx;
            interp0->base[1] = dtx;
            interp0->accum[0] = ty;
            interp0->base[0] = 0;
        #endif

        for(int cx = 0; cx < xe; cx++){
            #ifndef __unix__
                screen_buffer[fb_pos + cx] = pixels[interp_pop_full_result(interp0)];
            #else
                screen_buffer[fb_pos + cx] = pixels[((ty >> 16) << width_log2) + (tx >> 16)];
                tx += dtx;
            #endif
        }

        fb_pos += SCREEN_WIDTH;
        ty += dty;
    }
}


void engine_draw_fillrect_scale_trishear_viewport(uint16_t color, int32_t x, int32_t y, uint16_t width, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int32_t vx, int32_t vy, int32_t vw, int32_t vh){

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

void engine_draw_rect_scale_trishear_viewport(uint16_t color, int32_t x, int32_t y, uint16_t width, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int32_t vx, int32_t vy, int32_t vw, int32_t vh){

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
        if(cy == y || cy == y+ye) for(int cx = x; cx < x + xe; cx++) {
            xshift2 = (((yp >> 16) - y) * xsr2);
            int32_t xp = cx + (xshift >> 16) + (xshift2 >> 16);
            if(is_xy_inside_viewport(xp, yp >> 16, vx, vy, vw, vw)) screen_buffer[fb_pos + (cx) + ((yp >> 16) - cy) * SCREEN_WIDTH + (xshift2 >> 16)] = color;
            yp += ysr;
        } else {
            xshift2 = (((yp >> 16) - y) * xsr2);
            int32_t xp = x + (xshift >> 16) + (xshift2 >> 16);
            if(is_xy_inside_viewport(xp, yp >> 16, vx, vy, vw, vw)) screen_buffer[fb_pos + (x) + ((yp >> 16) - cy) * SCREEN_WIDTH + (xshift2 >> 16)] = color;
            yp += ysr*xe;

            xshift2 = (((yp >> 16) - y) * xsr2);
            xp = x+xe + (xshift >> 16) + (xshift2 >> 16);
            if(is_xy_inside_viewport(xp, yp >> 16, vx, vy, vw, vw)) screen_buffer[fb_pos + (x+xe) + ((yp >> 16) - cy) * SCREEN_WIDTH + (xshift2 >> 16)] = color;
        }

        fb_pos -= (xshift >> 16);
        fb_pos += SCREEN_WIDTH;
        xshift += xsr;
    }
}


void engine_draw_blit_scale_trishear(uint16_t *pixels, int32_t x, int32_t y, int32_t width, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int flip, uint16_t transparent_color){
    // #ifndef __unix__
    //     init_interp(width_log2);
    // #endif

    int32_t xe = (width * xsc) >> 16;
    int32_t ye = (height * ysc) >> 16;
    int32_t dtx = ((int64_t)width << 16) / xe;
    int32_t dty = ((int64_t)height << 16) / ye;
    int32_t ty = 0;
    int32_t tx = 0;

    if(xsc < 0){
        xe = -xe;
        x -= xe;
    }
    if(ysc < 0){
        ye = -ye;
        y -= ye;
        ty = (height << 16) - 0x10000;
    }

    int32_t fb_pos = y * SCREEN_WIDTH + x;
    int32_t x_start = ((xsc < 0)) ? ((width << 16) - 0x10000) : 0;

    int32_t xshift = 0;
    int32_t yshift = 0;
    int32_t xshift2 = 0;
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    for(int cy = 0; cy < ye; cy++){
        yshift = (xshift >> 16) * ysr;
        tx = x_start;
        fb_pos += (xshift >> 16);

        // #ifndef __unix__
        //     interp0->accum[1] = tx;
        //     interp0->base[1] = dtx;
        //     interp0->accum[0] = ty;
        //     interp0->base[0] = 0;
        // #endif

        if(flip) for(int cx = 0; cx < xe; cx++){
            xshift2 = ((cy + (yshift >> 16)) * xsr2);

            // #ifndef __unix__
            //     screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[width * height - 1 - interp_pop_full_result(interp0)];
            // #else
                uint32_t index = fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16);
                // ENGINE_FORCE_PRINTF("%ld", (xshift >> 16));
                if(index < SCREEN_BUFFER_SIZE_PIXELS){
                    uint16_t pixel = pixels[width * height - 1 - ((ty >> 16) * width + (tx >> 16))];
                    if(transparent_color == ENGINE_NO_TRANSPARENCY_COLOR || pixel != transparent_color){
                        screen_buffer[index] = pixel;
                    }
                }
                tx += dtx;
            // #endif

            yshift += ysr;
        }else for(int cx = 0; cx < xe; cx++){
            xshift2 = ((cy + (yshift >> 16)) * xsr2);

            // #ifndef __unix__
            //     screen_buffer[fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16)] = pixels[interp_pop_full_result(interp0)];
            // #else
                uint32_t index = fb_pos + (cx) + (yshift >> 16) * SCREEN_WIDTH + (xshift2 >> 16);
                // ENGINE_FORCE_PRINTF("%ld", (xshift >> 16));
                if(index < SCREEN_BUFFER_SIZE_PIXELS){
                    uint16_t pixel = pixels[(ty >> 16) * width + (tx >> 16)];
                    if(transparent_color == ENGINE_NO_TRANSPARENCY_COLOR || pixel != transparent_color){
                        screen_buffer[index] = pixel;
                    }
                }
                tx += dtx;
            // #endif

            yshift += ysr;
        }

        fb_pos -= (xshift >> 16);
        fb_pos += SCREEN_WIDTH;
        xshift += xsr;
        ty += dty;
    }
}


void engine_draw_blit_scale_rotate(uint16_t *pixels, float center_x, float center_y, uint16_t width, uint16_t height, float x_scale, float y_scale, float rotation_angle_rad, uint16_t transparent_color){
    
    ENGINE_PERFORMANCE_CYCLES_START();

    /*  https://cohost.org/tomforsyth/post/891823-rotation-with-three#:~:text=But%20the%20TL%3BDR%20is%20you%20do%20three%20shears%3A
        https://stackoverflow.com/questions/65909025/rotating-a-bitmap-with-3-shears    Lots of inspiration from here
        https://computergraphics.stackexchange.com/questions/10599/rotate-a-bitmap-with-shearing
        https://news.ycombinator.com/item?id=34485871 lots of sources
        https://graphicsinterface.org/wp-content/uploads/gi1986-15.pdf
        https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076
        https://datagenetics.com/blog/august32013/index.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html#:~:text=To%20do%20a%20shear%20operation%20on%20a%20raster%20image%20(that%20is%20to%20say%2C%20a%20bitmap)%2C%20we%20just%20shift%20all%20the%20pixels%20in%20a%20given%20row%20(column)%20by%20an%20easy%2Dto%2Dcalculate%20displacement

        The last link above highlights the most important part about doing rotations by shears:
        "To do a shear operation on a raster image (that is to say, a bitmap), we just shift all
        the pixels in a given row (column) by an easy-to-calculate displacement"

        As that link mentions, we'll do the rotation by doing three shears/displacements per-pixel per column.
        The displacements are performed twice on the x-axis and once on the y axis in x y x order.
    */

    // uint16_t *screen_buffer = engine_get_active_screen_buffer();

    // float scaled_width = (float)width * x_scale;
    // float scaled_height = (float)height * x_scale;

    // float scaled_half_width = scaled_width / 2.0f;
    // float scaled_half_height = scaled_height / 2.0f;

    // // Cache these calculations
    // float tri_shear_tan = tanf(rotation_angle_rad/2.0f);
    // float tri_shear_sin = sinf(rotation_angle_rad);
    
    // float sheared_x = 0;
    // float sheared_y = 0;

    // // Now we need to go over each column and displace the pixels
    // for(uint16_t src_pixel_x=0; src_pixel_x<width; src_pixel_x++){
    //     for(uint16_t src_pixel_y=0; src_pixel_y<height; src_pixel_y++){

    //         float src_pixel_centered_x = src_pixel_x - scaled_half_width;
    //         float src_pixel_centered_y = src_pixel_y - scaled_half_height;

    //         // Shear #1
    //         sheared_x = (float)(src_pixel_centered_x - src_pixel_centered_y * tri_shear_tan);
    //         sheared_y = src_pixel_centered_y;

    //         // Shear #2
    //         sheared_y = sheared_x * tri_shear_sin + sheared_y;

    //         // Shear #3
    //         sheared_x = sheared_x - sheared_y * tri_shear_tan;

    //         sheared_x = sheared_x * (scaled_width / width);
    //         sheared_y = sheared_y * (scaled_height / height);

    //         sheared_x += center_x;
    //         sheared_y += center_y;

    //         if((sheared_x >= 0 && sheared_x < SCREEN_WIDTH) && (sheared_y >= 0 && sheared_y < SCREEN_HEIGHT)){
    //             uint16_t src_pixel_index = src_pixel_y * width + src_pixel_x;
    //             uint16_t output_pixel_index = (uint16_t)sheared_y * SCREEN_WIDTH + (uint16_t)sheared_x;

    //             screen_buffer[output_pixel_index] = pixels[src_pixel_index];
    //         }
    //     }
    // }



    int32_t x = center_x;
    int32_t y = center_y;
    int32_t sprite_scale_x = (int32_t)(x_scale*65536+0.5);
    int32_t sprite_scale_y = (int32_t)(y_scale*65536+0.5);
    int16_t sprite_rotation = (int16_t)(rotation_angle_rad*1024.0f / (2.0f*PI));

    int flip = 0;

    // Step 1: Get theta inside (-pi/2, pi/2) and flip if we need to
    sprite_rotation &= 0x3FF;
    if(sprite_rotation > 0x200) sprite_rotation -= 0x400;
    if(sprite_rotation > 0x100){
        // flip = 1;
        sprite_rotation -= 0x200;
    } else if(sprite_rotation < -0x100){
        // flip = 1;
        sprite_rotation += 0x200;
    }

    int negative = 0;
    if(sprite_rotation < 0){
        negative = 1;
        sprite_rotation = -sprite_rotation;
    }

    int idx = (sprite_rotation << 1);
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
    int32_t xe = ((int64_t)width * sprite_scale_x) >> 16;
    int32_t ye = ((int64_t)height * sprite_scale_y) >> 16;
    if(sprite_scale_x < 0) xe = -xe;
    if(sprite_scale_y < 0) ye = -ye;
    int cx = ((int64_t)(xe/2) * c - (int64_t)(ye/2) * b) >> 16;
    int cy = ((int64_t)(ye/2) * c + (int64_t)(xe/2) * b) >> 16;
    if(sprite_scale_x < 0) cx -= xe;
    if(sprite_scale_y < 0) cy -= ye;

    // Step 4: Triple shear (a, b, a);
    engine_draw_blit_scale_trishear(pixels, x - cx, y - cy, width, height, sprite_scale_x, sprite_scale_y, a, b, a, flip, transparent_color);

    ENGINE_PERFORMANCE_CYCLES_STOP();
}


void engine_draw_fillrect_scale_rotate_viewport(uint16_t color, int32_t x, int32_t y, uint16_t width, uint16_t height, int32_t xsc, int32_t ysc, int16_t theta, int32_t vx, int32_t vy, int32_t vw, int32_t vh){
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