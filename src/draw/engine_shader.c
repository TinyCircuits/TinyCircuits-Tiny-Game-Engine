#include "engine_shader.h"
#include "draw/engine_color.h"

#include <string.h>
#include <stdlib.h>


// Fast function for when a node doesn't have any effects
uint16_t ENGINE_FAST_FUNCTION(engine_pixel_shader_empty)(uint16_t bg, uint16_t fg, float opacity, engine_shader_t *shader){
    return fg;
}


// Fast function for when a node only has opacity
uint16_t ENGINE_FAST_FUNCTION(engine_pixel_shader_alpha)(uint16_t bg, uint16_t fg, float opacity, engine_shader_t *shader){
    return engine_color_alpha_blend(bg, fg, opacity);
}


// Slow function for when a node has a custom shader (TODO: not implemented yet)
uint16_t ENGINE_FAST_FUNCTION(engine_pixel_shader_custom)(uint16_t bg, uint16_t fg, float opacity, engine_shader_t *shader){
    if(shader == NULL || shader->program == NULL || shader->program_len == 0){
        return fg;
    }

    uint16_t result = fg;
    uint8_t index = 0;

    while(index < shader->program_len){
        switch(shader->program[index]){
            case SHADER_OPACITY_BLEND:
            {
                result = engine_color_alpha_blend(bg, result, opacity);
            }
            break;
            case SHADER_RGB_INTERPOLATE:
            {
                uint16_t interpolate_to_color = 0x0000;
                interpolate_to_color |= (shader->program[index+1] << 8);
                interpolate_to_color |= (shader->program[index+2] << 0);

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


// Fast shader that can be passed from node draw callback to drawing functions to quickly paste pixels to buffer
engine_shader_t empty_shader = {
    .program = {},
    .program_len = 0,
    .execute = engine_pixel_shader_empty,
};


// Fast shader that can be passed from node draw callback to drawing functions to quickly paste pixels to buffer with opacity blending
engine_shader_t opacity_shader = {
    .program = {},
    .program_len = 0,
    .execute = engine_pixel_shader_alpha,
};