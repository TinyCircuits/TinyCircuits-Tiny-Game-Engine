#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"
#include <string.h>
#include <stdlib.h>

#include "nodes/node_base.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "draw/engine_shader.h"

#include "py/objstr.h"
#include "py/objtype.h"

// Defined in engine_display_common.c
extern uint16_t *active_screen_buffer;

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


void ENGINE_FAST_FUNCTION(engine_draw_pixel)(uint16_t color, int32_t x, int32_t y, float alpha, engine_shader_t *shader){
    if((x >= 0 && x < SCREEN_WIDTH) && (y >= 0 && y < SCREEN_HEIGHT)){
        uint16_t index = y * SCREEN_WIDTH + x;

        active_screen_buffer[index] = shader->execute(active_screen_buffer[index], color, alpha, shader);
    }
}


void ENGINE_FAST_FUNCTION(engine_draw_pixel_no_check)(uint16_t color, int32_t x, int32_t y, float alpha, engine_shader_t *shader){
    uint16_t index = y * SCREEN_WIDTH + x;
    active_screen_buffer[index] = shader->execute(active_screen_buffer[index], color, alpha, shader);
}


// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
void engine_draw_line(uint16_t color, float x_start, float y_start, float x_end, float y_end, mp_obj_t camera_node_base_in, float alpha, engine_shader_t *shader){
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

        engine_draw_pixel(color, (int32_t)line_x, (int32_t)line_y, alpha, shader);
    }
}




void engine_draw_blit(texture_resource_class_obj_t *texture, uint32_t offset, float center_x, float center_y, int32_t window_width, int32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha, engine_shader_t *shader){
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
    int32_t dim = (int32_t)sqrtf((scaled_window_width*scaled_window_width) + (scaled_window_height*scaled_window_height));
    float dim_half = (dim / 2.0f);

    // The top-left of the bitmap destination
    int32_t top_left_x = (int32_t)floorf(center_x - dim_half);
    int32_t top_left_y = (int32_t)floorf(center_y - dim_half);

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
                int32_t rotX = (int32_t)floorf(x);
                int32_t rotY = (int32_t)floorf(y);

                // If statements are expensive! Don't need to check if withing screen
                // bounds since those dimensions are clipped (destination rect)
                if((rotX >= 0 && rotX < window_width) && (rotY >= 0 && rotY < window_height)){
                    uint32_t src_offset = rotY * pixels_stride + rotX;
                    // uint16_t src_color = pixels[src_offset];
                    float src_alpha = 1.0f;
                    uint16_t src_color = texture->get_pixel(texture, offset+src_offset, &src_alpha);

                    if(src_color != transparent_color || src_color == ENGINE_NO_TRANSPARENCY_COLOR){
                        // active_screen_buffer[dest_offset] = shader->execute(active_screen_buffer[dest_offset], src_color, alpha, shader);
                        active_screen_buffer[dest_offset] = shader->execute(active_screen_buffer[dest_offset], src_color, alpha*src_alpha, shader);
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


void engine_draw_blit_depth(texture_resource_class_obj_t *texture, uint32_t offset, float center_x, float center_y, int32_t window_width, int32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha, uint16_t depth, engine_shader_t *shader){
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
    int32_t dim = (int32_t)sqrtf((scaled_window_width*scaled_window_width) + (scaled_window_height*scaled_window_height));
    float dim_half = (dim / 2.0f);

    // The top-left of the bitmap destination
    int32_t top_left_x = (int32_t)floorf(center_x - dim_half);
    int32_t top_left_y = (int32_t)floorf(center_y - dim_half);

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
                int32_t rotX = (int32_t)floorf(x);
                int32_t rotY = (int32_t)floorf(y);

                // If statements are expensive! Don't need to check if withing screen
                // bounds since those dimensions are clipped (destination rect)
                if((rotX >= 0 && rotX < window_width) && (rotY >= 0 && rotY < window_height)){
                    uint32_t src_offset = rotY * pixels_stride + rotX;
                    // uint16_t src_color = pixels[src_offset];
                    float src_alpha = 1.0f;
                    uint16_t src_color = texture->get_pixel(texture, offset+src_offset, &src_alpha);

                    if(src_color != transparent_color || src_color == ENGINE_NO_TRANSPARENCY_COLOR){
                        if(engine_display_store_check_depth_index(dest_offset, depth)){
                            active_screen_buffer[dest_offset] = shader->execute(active_screen_buffer[dest_offset], src_color, alpha*src_alpha, shader);
                        }
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


void engine_draw_rect(uint16_t color, float center_x, float center_y, int32_t width, int32_t height, float x_scale, float y_scale, float rotation_radians, float alpha, engine_shader_t *shader){
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
    int32_t dim = (int32_t)sqrtf((scaled_width*scaled_width) + (scaled_height*scaled_height));
    float dim_half = (dim / 2.0f);

    // The top-left of the bitmap destination
    int32_t top_left_x = (int32_t)floorf(center_x - dim_half);
    int32_t top_left_y = (int32_t)floorf(center_y - dim_half);

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
                int32_t rotX = (int32_t)floorf(x);
                int32_t rotY = (int32_t)floorf(y);

                // If statements are expensive! Don't need to check if withing screen
                // bounds since those dimensions are clipped (destination rect)
                if((rotX >= 0 && rotX < width) && (rotY >= 0 && rotY < height)){
                    active_screen_buffer[dest_offset] = shader->execute(active_screen_buffer[dest_offset], color, alpha, shader);
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


void engine_draw_outline_circle(uint16_t color, float center_x, float center_y, float radius, float alpha, engine_shader_t *shader){
    // https://stackoverflow.com/a/58629898
    float distance = radius;
    float angle_increment = acosf(1 - 1/distance) * 2.0f;   // Multiply by 2.0 since care about speed and not accuracy as much

    for(float angle = 0; angle <= 90; angle += angle_increment){
        float cx = distance * cosf(angle);
        float cy = distance * sinf(angle);
        
        // Bottom right quadrant of the circle
        int brx = (int)(center_x+cx);
        int bry = (int)(center_y+cy);

        // Bottom left quadrant of the circle
        int blx = (int)(center_x-cx);
        int bly = (int)(center_y+cy);

        // Top right quadrant of the circle
        int trx = (int)(center_x+cx);
        int try = (int)(center_y-cy);

        // Top left quadrant of the circle
        int tlx = (int)(center_x-cx);
        int tly = (int)(center_y-cy);

        engine_draw_pixel(color, brx, bry, alpha, shader);
        engine_draw_pixel(color, blx, bly, alpha, shader);
        engine_draw_pixel(color, trx, try, alpha, shader);
        engine_draw_pixel(color, tlx, tly, alpha, shader);
    }
}


void engine_draw_filled_circle(uint16_t color, float center_x, float center_y, float radius, float alpha, engine_shader_t *shader){
    float radius_sqr = radius * radius;
    int x_min = (int)(-radius);
    int x_max = (int)radius;

    // https://stackoverflow.com/a/59211338
    for(int x=x_min; x<x_max; x++){
        int hh = (int)sqrt(radius_sqr - x * x);
        int rx = (int)center_x + x;
        int ph = (int)center_y + hh;

        for(int y=(int)center_y-hh; y<ph; y++){
            engine_draw_pixel(color, rx, y, alpha, shader);
        }
    }
}


void engine_draw_text(font_resource_class_obj_t *font, mp_obj_t text, float center_x, float center_y, float text_box_width, float text_box_height, float letter_spacing, float line_spacing, float x_scale, float y_scale, float rotation_radians, float alpha, engine_shader_t *shader){    
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

        // Replace any character that's not a newline or is
        // not otherwise printable with a question mark `?`
        if(current_char != 10 && (current_char < 32 || current_char > 126)){
            current_char = 63;
        }

        // Check if newline, otherwise any other character contributes to text box width
        if(current_char == 10){
            // Move to start of line
            char_x -= (cos_angle * current_row_width * x_scale);
            char_y += (sin_angle * current_row_width * x_scale);

            // Move to next line
            char_x -= (cos_angle_perp * (font->glyph_height+line_spacing) * y_scale);
            char_y += (sin_angle_perp * (font->glyph_height+line_spacing) * y_scale);

            current_row_width = 0.0f;
            continue;
        }

        // The width of this character, all heights are defined by bitmap font height-1
        const uint8_t char_width = font_resource_get_glyph_width(font, current_char);
        const float char_width_half = (char_width * 0.5f) + letter_spacing;

        float final_char_x = char_x;
        float final_char_y = char_y;

        final_char_x += (cos_angle * char_width_half * x_scale);
        final_char_y -= (sin_angle * char_width_half * x_scale);

        // Offset inside the ASCII font bitmap (not into where we're drawing)
        uint16_t char_bitmap_x_offset = font_resource_get_glyph_x_offset(font, current_char);

        engine_draw_blit(font->texture_resource, engine_math_2d_to_1d_index(char_bitmap_x_offset, 0, font->texture_resource->pixel_stride),
                        floorf(final_char_x), floorf(final_char_y),
                        char_width, font->glyph_height,
                        font->texture_resource->pixel_stride,
                        x_scale,
                        y_scale,
                        -rotation_radians,
                        0,
                        alpha,
                        shader);

        // Move to next character position in row
        char_x += (cos_angle * (char_width+letter_spacing) * x_scale);
        char_y -= (sin_angle * (char_width+letter_spacing) * x_scale);

        current_row_width += char_width+letter_spacing;
    }
}


// // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/#:~:text=int%20x%2C%20y%3B%0A%7D%3B-,int%20orient2d,-(const%20Point2D%26%20a
// int32_t orient2d(float x0, float y0, float x1, float y1, float x2, float y2){
//     return (x1-x0)*(y2-y0) - (y1-y0)*(x2-x0);
// }


// https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/#:~:text=int%20x%2C%20y%3B%0A%7D%3B-,int%20orient2d,-(const%20Point2D%26%20a
// https://jtsorlinis.github.io/rendering-tutorial/#:~:text=Here%27s%20what%20that%20looks%20like%20in%20code%3A
// https://jtsorlinis.github.io/rendering-tutorial/#:~:text=this%20triangle%0A%7D-,Back%20to%20business,-So%2C%20why%20is
// This returns twice the area of the triangle made of of these points. This is used
// to determine if a point is inside a triangle. For example, if we let the point
// to be checked to be x2, and y2, then the point is on the right side of the triangle
// when the twice area is positive and outside if negative. If we test that the point in
// x2 and y2 is positive for all three edges, the point is on the right side of the edge
// and inside the triangle (assuming we are going for a right/clock-wise of vertices, want
// point to be on left side of edges if using a left/counter clock-wise winding)
float edge_function(float x0, float y0, float x1, float y1, float x2, float y2){
    return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
}


// https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/#:~:text=trivial%20to%20traverse.-,This%20gives%3A,-void%20drawTri(const
// https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/#:~:text=In%20our%20basic%20triangle%20rasterization%20loop
void engine_draw_filled_triangle(uint16_t color, float x0, float y0, float x1, float y1, float x2, float y2, float alpha, engine_shader_t *shader){
    // // Triangle setup
    // int16_t A01 = (int16_t)(y0 - y1), B01 = (int16_t)(x1 - x0);
    // int16_t A12 = (int16_t)(y1 - y2), B12 = (int16_t)(x2 - x1);
    // int16_t A20 = (int16_t)(y2 - y0), B20 = (int16_t)(x0 - x2);

    // // Compute triangle bounding box
    // int16_t minX = (int16_t)min3(x0, x1, x2);
    // int16_t minY = (int16_t)min3(y0, y1, y2);
    // int16_t maxX = (int16_t)max3(x0, x1, x2);
    // int16_t maxY = (int16_t)max3(y0, y1, y2);

    // // Clip against screen bounds
    // minX = max(minX, 0);
    // minY = max(minY, 0);
    // maxX = min(maxX, SCREEN_WIDTH_MINUS_1);
    // maxY = min(maxY, SCREEN_HEIGHT_MINUS_1);

    // int16_t px = minX;
    // int16_t py = minY;

    // int16_t w0_row = (int16_t)orient2d(x1, y1, x2, y2, px, py);
    // int16_t w1_row = (int16_t)orient2d(x2, y2, x0, y0, px, py);
    // int16_t w2_row = (int16_t)orient2d(x0, y0, x1, y1, px, py);

    // // Rasterize
    // for(py = minY; py <= maxY; py++){
    //     // Barycentric coordinates at start of row
    //     int16_t w0 = w0_row;
    //     int16_t w1 = w1_row;
    //     int16_t w2 = w2_row;

    //     for(px = minX; px <= maxX; px++){
    //         // If p is on or inside all edges, render pixel.
    //         // https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/#:~:text=if%20((w0%20%7C%20w1%20%7C%20w2)%20%3E%3D%200)
    //         if ((w0 | w1 | w2) >= 0){
    //             // if(engine_display_store_check_depth(px, py, depth)){
    //                 engine_draw_pixel_no_check(color, px, py, alpha, shader);
    //             // }
    //         }

    //         // One step to the right
    //         w0 += A12;
    //         w1 += A20;
    //         w2 += A01;
    //     }

    //     // One row step
    //     w0_row += B12;
    //     w1_row += B20;
    //     w2_row += B01;
    // }
}


void engine_draw_filled_triangle_depth(texture_resource_class_obj_t *texture, uint16_t color,
                                       float ax, float ay, uint16_t depth_az, float au, float av,
                                       float bx, float by, uint16_t depth_bz, float bu, float bv,
                                       float cx, float cy, uint16_t depth_cz, float cu, float cv,
                                       float w0, float w1, float w2,
                                       float alpha, engine_shader_t *shader){
    // A = x0, y0
    // B = x1, y1
    // C = x2, y2
    const float ABC = edge_function(ax, ay, bx, by, cx, cy);

    // Do not render triangles with 2x negative area - back face culling
    // https://jtsorlinis.github.io/rendering-tutorial/#:~:text=RESET-,A%20nifty%20trick,-Another%20really%20useful
    if(ABC <= 0.0f){
        // Do not draw this triangle
        return;
    }

    // https://jtsorlinis.github.io/rendering-tutorial/#:~:text=the%20triangle%27s%20vertices
    // Compute triangle bounding box. Each pixel in this box will be
    // determined to be inside or outside of the triangle
    int32_t min_x = (int32_t)min3(ax, bx, cx);
    int32_t min_y = (int32_t)min3(ay, by, cy);
    int32_t max_x = (int32_t)max3(ax, bx, cx);
    int32_t max_y = (int32_t)max3(ay, by, cy);

    // Clip against screen bounds (added this). Don't want to
    // check if pixels are inside the triangle if not visible
    min_x = max(min_x, 0);
    min_y = max(min_y, 0);
    max_x = min(max_x, SCREEN_WIDTH_MINUS_1);
    max_y = min(max_y, SCREEN_HEIGHT_MINUS_1);

    // Start at the minimum x and y corner of the triangle view box
    int16_t px = (int16_t)min_x;
    int16_t py = (int16_t)min_y;

    // https://jtsorlinis.github.io/rendering-tutorial/#:~:text=this%20triangle%0A%7D-,Back%20to%20business,-So%2C%20why%20is
    // https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/#:~:text=In%20our%20basic%20triangle%20rasterization%20loop
    // Calculate our edge functions. If (px, py) is on the
    // right side of all of the edges, each of these will
    // be a positive number
    float BCP_ROW = edge_function(bx, by, cx, cy, px, py) / ABC;
    float CAP_ROW = edge_function(cx, cy, ax, ay, px, py) / ABC;
    float ABP_ROW = edge_function(ax, ay, bx, by, px, py) / ABC;

    // https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/#:~:text=In%20our%20basic%20triangle%20rasterization%20loop
    float dy_bc = (float)(by - cy) / ABC;
    float dx_bc = (float)(cx - bx) / ABC;
    
    float dy_ca = (float)(cy - ay) / ABC;
    float dx_ca = (float)(ax - cx) / ABC;

    float dy_ab = (float)(ay - by) / ABC;
    float dx_ab = (float)(bx - ax) / ABC;


    // Go through all pixels in triangle view box and check if each
    // point is inside or outside the triangle inside the box
    for(py=min_y; py<=max_y; py++){
        // Barycentric coordinates at start of row
        float BCP = BCP_ROW;
        float CAP = CAP_ROW;
        float ABP = ABP_ROW;

        for(px=min_x; px<=max_x; px++){

            // https://jtsorlinis.github.io/rendering-tutorial/#:~:text=get%20the%20interpolated%20colour
            // BCP + CAP + ABP = 1.0
            uint16_t depth_p = (uint16_t)((float)depth_az*BCP + (float)depth_bz*CAP + (float)depth_cz*ABP);

            // Check that the pixel is on the right side of each
            // edge for all the edge functions calculated. Instead of
            // comparing directly to 0.0, make sure triangles get filled
            // by comparing to numbers above some small negative number
            if((ABP >= -0.001f && BCP >= -0.001f && CAP >= -0.001f) && engine_display_store_check_depth(px, py, depth_p)){

                // https://stackoverflow.com/questions/12360023/barycentric-coordinates-texture-mapping
                // https://computergraphics.stackexchange.com/a/4091
                // https://www.reddit.com/r/opengl/comments/49fdhc/comment/d0rlh12/
                // https://web.archive.org/web/20240416044207/https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes.html
                float w = BCP/w0 + CAP/w1 + ABP/w2;

                uint16_t u = (uint16_t)((BCP*au/w0 + CAP*bu/w1 + ABP*cu/w2) / w);
                uint16_t v = (uint16_t)((BCP*av/w0 + CAP*bv/w1 + ABP*cv/w2) / w);

                // // https://computergraphics.stackexchange.com/questions/4079/perspective-correct-texture-mapping
                // float tau = au / w0;
                // float tav = av / w0;
                // float tinvw0 = 1.0f / w0;

                // float tbu = bu / w1;
                // float tbv = bv / w1;
                // float tinvw1 = 1.0f / w1;

                // float tcu = cu / w2;
                // float tcv = cv / w2;
                // float tinvw2 = 1.0f / w2;

                // // float invw = 1.0f / w;


                // uint16_t invw = (uint16_t)((tinvw0*BCP + tinvw1*CAP + tinvw2*ABP));
                // uint16_t u = (uint16_t)((tau*BCP + tbu*CAP + tcu*ABP)/(1.0f/invw));
                // uint16_t v = (uint16_t)((tav*BCP + tbv*CAP + tcv*ABP)/(1.0f/invw));

                // uint16_t u = (uint16_t)((au*BCP + bu*CAP + cu*ABP));
                // uint16_t v = (uint16_t)((av*BCP + bv*CAP + cv*ABP));

                // Get the pixel from the texture
                uint32_t index = v * texture->width + u;
                float texture_pixel_alpha = 0.0f;

                uint16_t texture_pixel_color = texture->get_pixel(texture, index, &texture_pixel_alpha);

                // Mix
                alpha = alpha * texture_pixel_alpha;

                engine_draw_pixel_no_check(texture_pixel_color, px, py, alpha, shader);
            }

            // One step to the right
            BCP += dy_bc;
            CAP += dy_ca;
            ABP += dy_ab;
        }

        // One row step
        BCP_ROW += dx_bc;
        CAP_ROW += dx_ca;
        ABP_ROW += dx_ab;
    }
}