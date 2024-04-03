#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"
#include "math/trig_tables.h"
#include "utility/engine_defines.h"
#include <string.h>
#include <stdlib.h>

#include "nodes/node_base.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"

#include "py/objstr.h"
#include "py/objtype.h"


uint16_t ENGINE_FAST_FUNCTION(engine_pixel_shader)(uint16_t bg, uint16_t fg, float opacity, uint8_t *shader, uint8_t shader_len){
    uint16_t result = fg;
    uint8_t index = 0;

    while(index < shader_len){
        switch(shader[index]){
            case SHADER_OPACITY_BLEND:
            {
                result = engine_color_alpha_blend(bg, result, opacity);
            }
            break;
            case SHADER_RGB_INTERPOLATE:
            {
                uint16_t interpolate_to_color = 0x0000;
                interpolate_to_color |= (shader[index+1] << 8);
                interpolate_to_color |= (shader[index+2] << 0);

                float t = 0;
                memcpy(&t, shader+index+3, 4);

                result = engine_color_blend(result, interpolate_to_color, t);

                index += 6; // OPPFFFF
                continue;
            }
            break;
        }

        index++;
    }

    return result;
}


void ENGINE_FAST_FUNCTION(engine_draw_fill_color)(uint16_t color, uint16_t *screen_buffer){
    uint16_t *buf = screen_buffer;
    uint16_t count = SCREEN_BUFFER_SIZE_PIXELS;

    while(count--) *buf++ = color;
}


void ENGINE_FAST_FUNCTION(engine_draw_fill_buffer)(uint16_t* src_buffer, uint16_t *screen_buffer){
    uint16_t index = 0;

    while(index < SCREEN_BUFFER_SIZE_PIXELS){
        screen_buffer[index] = src_buffer[index];
        index++;
    }
}


void ENGINE_FAST_FUNCTION(engine_draw_pixel)(uint16_t color, int32_t x, int32_t y, float alpha){
    if((x >= 0 && x < SCREEN_WIDTH) && (y >= 0 && y < SCREEN_HEIGHT)){
        uint16_t *screen_buffer = engine_get_active_screen_buffer();
        uint16_t index = y * SCREEN_WIDTH + x;

        screen_buffer[index] = engine_color_alpha_blend(screen_buffer[index], color, alpha);
    }
}


// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
void engine_draw_line(uint16_t color, float x_start, float y_start, float x_end, float y_end, mp_obj_t camera_node_base_in, float alpha){
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

        engine_draw_pixel(color, (int32_t)line_x, (int32_t)line_y, alpha);
    }
}




void engine_draw_blit(uint16_t *pixels, float center_x, float center_y, uint32_t window_width, uint32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha){
    /*  https://cohost.org/tomforsyth/post/891823-rotation-with-three#:~:text=But%20the%20TL%3BDR%20is%20you%20do%20three%20shears%3A
        https://stackoverflow.com/questions/65909025/rotating-a-bitmap-with-3-shears    Lots of inspiration from here
        https://computergraphics.stackexchange.com/questions/10599/rotate-a-bitmap-with-shearing
        https://news.ycombinator.com/item?id=34485871 lots of sources
        https://graphicsinterface.org/wp-content/uploads/gi1986-15.pdf
        https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076
        https://datagenetics.com/blog/august32013/index.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html#:~:text=To%20do%20a%20shear%20operation%20on%20a%20raster%20image%20(that%20is%20to%20say%2C%20a%20bitmap)%2C%20we%20just%20shift%20all%20the%20pixels%20in%20a%20given%20row%20(column)%20by%20an%20easy%2Dto%2Dcalculate%20displacement

        https://codereview.stackexchange.com/a/86546 <- Not trishear but might be good enough, it's what is used below

        The last link above highlights the most important part about doing rotations by shears:
        "To do a shear operation on a raster image (that is to say, a bitmap), we just shift all
        the pixels in a given row (column) by an easy-to-calculate displacement"

        As that link mentions, we'll do the rotation by doing three shears/displacements per-pixel per column.
        The displacements are performed twice on the x-axis and once on the y axis in x y x order.
    */

    // ENGINE_PERFORMANCE_CYCLES_START();
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    float inverse_x_scale = 1.0f / x_scale;
    float inverse_y_scale = 1.0f / y_scale;
    
    // https://codereview.stackexchange.com/a/86546
    float sin_angle = sinf(rotation_radians);
    float cos_angle = cosf(rotation_radians);

    // Used to traverse about rotation
    float sin_angle_inv_scaled = sin_angle * inverse_y_scale;
    float cos_angle_inv_scaled = cos_angle * inverse_x_scale;

    // Controls the scale of the destination rectangle,
    // which in turn defines the total scale of the bitmap
    float scaled_window_width = window_width * x_scale;
    float scaled_window_height = window_height * y_scale;

    float half_scaled_window_width = scaled_window_width * 0.5f;
    float half_scaled_window_height = scaled_window_height * 0.5f;

    // When rotated at 45 degrees, make sure corners don't get cut
    // off: https://math.stackexchange.com/questions/2915935/radius-of-a-circle-touching-a-rectangle-both-of-which-are-inside-a-square
    uint32_t dim = (uint32_t)sqrtf((scaled_window_width*scaled_window_width) + (scaled_window_height*scaled_window_height));
    float dim_half = (dim / 2.0f);

    // The top-left of the bitmap destination
    int32_t top_left_x = center_x - dim_half;
    int32_t top_left_y = center_y - dim_half;

    // If the top-left is above the viewport but
    // the bitmap may eventually showup, clip the
    // top of the destination rectangle
    int32_t j_start = 0;
    if(top_left_y < 0){
        j_start = abs(top_left_y);
    }

    // If the top-left is left of the viewport
    // but the bitmap may eventually showup, clip
    // the left of the destination rectangle
    int32_t i_start = 0;
    if(top_left_x < 0){
        i_start = abs(top_left_x);
    }

    // 1D index into the screen buffer of where the bitmap will go
    const uint32_t center_pixel_offset = (top_left_y+j_start) * SCREEN_WIDTH + (top_left_x+i_start);

    // Used for tracking where we are in the screen_buffer
    uint32_t dest_offset = center_pixel_offset;

    // The stride to the next row usually consists of just the
    // screen width, however, subtract the dimensions of the 
    // destination rect so that we end up at the next row
    // in the rect. Also proceed forward on the next row
    // by the amount we are left clipping the dest rect
    uint32_t next_dest_row_offset = SCREEN_WIDTH - dim + i_start;

    int32_t i, j;

    // Start from clipped top and go until max destination rectangle
    // height (bounding-box) or until the start drawing out of bounds
    // (clip bottom)
    for(j=j_start; j<dim && top_left_y+j < SCREEN_HEIGHT; j++){
        // Center inside destination rectangle.
        // Offset where we are in the src bitmap
        // by left-clip amount ('i_start')
        float deltaY = j - dim_half;
        float deltaX = 0 - dim_half + i_start;

        // Calculate the location of the SRC pixel. If the destination
        // gets scaled larger then we need to inversely scale into the
        // src, and vice versa
        float x = (half_scaled_window_width + deltaX * cos_angle + deltaY * sin_angle) * inverse_x_scale;
        float y = (half_scaled_window_height - deltaX * sin_angle + deltaY * cos_angle) * inverse_y_scale;

        for(i=i_start; i<dim; i++){
            // Check if drawing to draw out of bounds to the right,
            // if so, stop drawing the destination row early and
            // move on to the next
            if(top_left_x+i < SCREEN_WIDTH){
                // Uncomment to see background. Drawing
                // sprites that are thin could be optimized
                // screen_buffer[dest_offset] = 0b11111000000000;

                // Floor these otherwise get artifacts (don't exactly know why).
                // Floor + int seems to be faster than comparing floats
                int32_t rotX = floorf(x);
                int32_t rotY = floorf(y);

                // If statements are expensive! Don't need to check if withing screen
                // bounds since those dimensions are clipped (destination rect)
                if((rotX >= 0 && rotX < window_width) && (rotY >= 0 && rotY < window_height)){
                    uint32_t src_offset = rotY * pixels_stride + rotX;
                    uint16_t src_color = pixels[src_offset];

                    if(src_color != transparent_color || src_color == ENGINE_NO_TRANSPARENCY_COLOR){
                        screen_buffer[dest_offset] = engine_pixel_shader(screen_buffer[dest_offset], src_color, alpha, (uint8_t[]){SHADER_OPACITY_BLEND}, 1);
                    }
                }

                // While in row, keep traversing about rotation
                x += cos_angle_inv_scaled;
                y -= sin_angle_inv_scaled;

                // Go to next pixel next time to set it
                dest_offset += 1;
            }else{
                // After this will be a subtraction by 'dim'
                // but we need to be at the end of the row
                // for that to work, set 'dest_offset' to end
                // See 'next_dest_row_offset'
                dest_offset += dim-i;
                break;
            }
        }

        // Go to next row but at the left of it (SCREEN_WIDTH - dim)
        dest_offset += next_dest_row_offset;
    }

    // ENGINE_PERFORMANCE_CYCLES_STOP();
}

void engine_draw_rect(uint16_t color, float center_x, float center_y, uint32_t width, uint32_t height, float x_scale, float y_scale, float rotation_radians, float alpha){
    /*  https://cohost.org/tomforsyth/post/891823-rotation-with-three#:~:text=But%20the%20TL%3BDR%20is%20you%20do%20three%20shears%3A
        https://stackoverflow.com/questions/65909025/rotating-a-bitmap-with-3-shears    Lots of inspiration from here
        https://computergraphics.stackexchange.com/questions/10599/rotate-a-bitmap-with-shearing
        https://news.ycombinator.com/item?id=34485871 lots of sources
        https://graphicsinterface.org/wp-content/uploads/gi1986-15.pdf
        https://gautamnagrawal.medium.com/rotating-image-by-any-angle-shear-transformation-using-only-numpy-d28d16eb5076
        https://datagenetics.com/blog/august32013/index.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html
        https://www.ocf.berkeley.edu/~fricke/projects/israel/paeth/rotation_by_shearing.html#:~:text=To%20do%20a%20shear%20operation%20on%20a%20raster%20image%20(that%20is%20to%20say%2C%20a%20bitmap)%2C%20we%20just%20shift%20all%20the%20pixels%20in%20a%20given%20row%20(column)%20by%20an%20easy%2Dto%2Dcalculate%20displacement

        https://codereview.stackexchange.com/a/86546 <- Not trishear but might be good enough, it's what is used below

        The last link above highlights the most important part about doing rotations by shears:
        "To do a shear operation on a raster image (that is to say, a bitmap), we just shift all
        the pixels in a given row (column) by an easy-to-calculate displacement"

        As that link mentions, we'll do the rotation by doing three shears/displacements per-pixel per column.
        The displacements are performed twice on the x-axis and once on the y axis in x y x order.
    */

    // ENGINE_PERFORMANCE_CYCLES_START();
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    float inverse_x_scale = 1.0f / x_scale;
    float inverse_y_scale = 1.0f / y_scale;
    
    // https://codereview.stackexchange.com/a/86546
    float sin_angle = sinf(rotation_radians);
    float cos_angle = cosf(rotation_radians);

    // Used to traverse about rotation
    float sin_angle_inv_scaled = sin_angle * inverse_y_scale;
    float cos_angle_inv_scaled = cos_angle * inverse_x_scale;

    // Controls the scale of the destination rectangle,
    // which in turn defines the total scale of the bitmap
    float scaled_width = width * x_scale;
    float scaled_height = height * y_scale;

    float half_scaled_width = scaled_width * 0.5f;
    float half_scaled_height = scaled_height * 0.5f;

    // When rotated at 45 degrees, make sure corners don't get cut
    // off: https://math.stackexchange.com/questions/2915935/radius-of-a-circle-touching-a-rectangle-both-of-which-are-inside-a-square
    uint32_t dim = (uint32_t)sqrtf((scaled_width*scaled_width) + (scaled_height*scaled_height));
    float dim_half = (dim / 2.0f);

    // The top-left of the bitmap destination
    int32_t top_left_x = center_x - dim_half;
    int32_t top_left_y = center_y - dim_half;

    // If the top-left is above the viewport but
    // the bitmap may eventually showup, clip the
    // top of the destination rectangle
    int32_t j_start = 0;
    if(top_left_y < 0){
        j_start = abs(top_left_y);
    }

    // If the top-left is left of the viewport
    // but the bitmap may eventually showup, clip
    // the left of the destination rectangle
    int32_t i_start = 0;
    if(top_left_x < 0){
        i_start = abs(top_left_x);
    }

    // 1D index into the screen buffer of where the bitmap will go
    const uint32_t center_pixel_offset = (top_left_y+j_start) * SCREEN_WIDTH + (top_left_x+i_start);

    // Used for tracking where we are in the screen_buffer
    uint32_t dest_offset = center_pixel_offset;

    // The stride to the next row usually consists of just the
    // screen width, however, subtract the dimensions of the 
    // destination rect so that we end up at the next row
    // in the rect. Also proceed forward on the next row
    // by the amount we are left clipping the dest rect
    uint32_t next_dest_row_offset = SCREEN_WIDTH - dim + i_start;

    int32_t i, j;

    // Start from clipped top and go until max destination rectangle
    // height (bounding-box) or until the start drawing out of bounds
    // (clip bottom)
    for(j=j_start; j<dim && top_left_y+j < SCREEN_HEIGHT; j++){
        // Center inside destination rectangle.
        // Offset where we are in the src bitmap
        // by left-clip amount ('i_start')
        float deltaY = j - dim_half;
        float deltaX = 0 - dim_half + i_start;

        // Calculate the location of the SRC pixel. If the destination
        // gets scaled larger then we need to inversely scale into the
        // src, and vice versa
        float x = (half_scaled_width + deltaX * cos_angle + deltaY * sin_angle) * inverse_x_scale;
        float y = (half_scaled_height - deltaX * sin_angle + deltaY * cos_angle) * inverse_y_scale;

        for(i=i_start; i<dim; i++){
            // Check if drawing to draw out of bounds to the right,
            // if so, stop drawing the destination row early and
            // move on to the next
            if(top_left_x+i < SCREEN_WIDTH){
                // Uncomment to see background. Drawing
                // sprites that are thin could be optimized
                // screen_buffer[dest_offset] = 0b11111000000000;

                // Floor these otherwise get artifacts (don't exactly know why).
                // Floor + int seems to be faster than comparing floats
                int32_t rotX = floorf(x);
                int32_t rotY = floorf(y);

                // If statements are expensive! Don't need to check if withing screen
                // bounds since those dimensions are clipped (destination rect)
                if((rotX >= 0 && rotX < width) && (rotY >= 0 && rotY < height)){
                    screen_buffer[dest_offset] = engine_color_alpha_blend(screen_buffer[dest_offset], color, alpha);
                }

                // While in row, keep traversing about rotation
                x += cos_angle_inv_scaled;
                y -= sin_angle_inv_scaled;

                // Go to next pixel next time to set it
                dest_offset += 1;
            }else{
                // After this will be a subtraction by 'dim'
                // but we need to be at the end of the row
                // for that to work, set 'dest_offset' to end
                // See 'next_dest_row_offset'
                dest_offset += dim-i;
                break;
            }
        }

        // Go to next row but at the left of it (SCREEN_WIDTH - dim)
        dest_offset += next_dest_row_offset;
    }

    // ENGINE_PERFORMANCE_CYCLES_STOP();
}


void engine_draw_text(font_resource_class_obj_t *font, mp_obj_t text, float center_x, float center_y, float text_box_width, float text_box_height, float x_scale, float y_scale, float rotation_radians, float alpha){    
    float sin_angle = sinf(rotation_radians);
    float cos_angle = cosf(rotation_radians);

    float sin_angle_perp = sinf(rotation_radians + HALF_PI);
    float cos_angle_perp = cosf(rotation_radians + HALF_PI);

    // Since sprites are centered by default and the text box height includes the
    // height of the first line, get rid of one line's worth of height to center
    // it correctly
    float text_box_width_half = text_box_width * 0.5f;
    float text_box_height_half = (text_box_height - font->glyph_height) * 0.5f;

    // Set starting point to text box origin then translate to
    // starting in top-left by column and row shifts (while rotated).
    // This way, the text box rotates about its origin position set
    // by the user
    float char_x = center_x;
    float char_y = center_y;

    // Move to left (go left and then up)
    char_x -= (cos_angle * text_box_width_half * x_scale);
    char_y += (sin_angle * text_box_width_half * x_scale);

    // Move up
    char_x += (cos_angle_perp * text_box_height_half * y_scale);
    char_y -= (sin_angle_perp * text_box_height_half * y_scale);

    float current_row_width = 0.0f;

    // Get length of string: https://github.com/v923z/micropython-usermod/blob/master/snippets/stringarg/stringarg.c
    GET_STR_DATA_LEN(text, str, str_len);

    for(uint16_t icx=0; icx<str_len; icx++){
        char current_char = ((char *)str)[icx];

        // Check if newline, otherwise any other character contributes to text box width
        if(current_char == 10){
            // Move to start of line
            char_x -= (cos_angle * current_row_width * x_scale);
            char_y += (sin_angle * current_row_width * x_scale);

            // Move to next line
            char_x -= (cos_angle_perp * font->glyph_height * y_scale);
            char_y += (sin_angle_perp * font->glyph_height * y_scale);

            current_row_width = 0.0f;
            continue;
        }

        // The width of this character, all heights are defined by bitmap font height-1
        const uint8_t char_width = font_resource_get_glyph_width(font, current_char);
        const float char_width_half = char_width * 0.5f;

        float final_char_x = char_x;
        float final_char_y = char_y;

        final_char_x += (cos_angle * char_width_half * x_scale);
        final_char_y -= (sin_angle * char_width_half * x_scale);

        // Offset inside the ASCII font bitmap (not into where we're drawing)
        uint16_t char_bitmap_x_offset = font_resource_get_glyph_x_offset(font, current_char);

        engine_draw_blit(font->texture_resource->data+engine_math_2d_to_1d_index(char_bitmap_x_offset, 0, font->texture_resource->width),
                        (final_char_x), (final_char_y),
                        char_width, font->glyph_height,
                        font->texture_resource->width,
                        x_scale,
                        y_scale,
                        -rotation_radians,
                        0,
                        alpha);

        // Move to next character position in row
        char_x += (cos_angle * char_width * x_scale);
        char_y -= (sin_angle * char_width * x_scale);

        current_row_width += char_width;
    }
}