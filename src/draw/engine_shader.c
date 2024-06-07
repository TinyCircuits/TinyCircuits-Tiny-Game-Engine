#include "engine_shader.h"
#include "draw/engine_color.h"
#include "debug/debug_print.h"

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
    uint8_t index = 0;

    while(index < shader->program_len){
        switch(shader->program[index]){
            case SHADER_OPACITY_BLEND:
            {
                fg = engine_color_alpha_blend(bg, fg, opacity);
            }
            break;
            case SHADER_RGB_INTERPOLATE:
            {
                uint16_t interpolate_to_color = 0x0000;
                interpolate_to_color |= (shader->program[index+1] << 8);
                interpolate_to_color |= (shader->program[index+2] << 0);

                float t = 0;
                memcpy(&t, shader->program+index+3, sizeof(float));

                fg = engine_color_blend(fg, interpolate_to_color, t);

                index += 6; // OPPFFFF
                continue;
            }
            break;
        }

        index++;
    }

    return fg;
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


engine_shader_t blend_opacity_shader = {
    .program = {SHADER_RGB_INTERPOLATE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, SHADER_OPACITY_BLEND},
    .program_len = 8,
    .execute = engine_pixel_shader_custom,
};

engine_shader_t *builtin_shaders[3] = {
    &empty_shader,
    &opacity_shader,
    &blend_opacity_shader
};

engine_shader_t *engine_get_builtin_shader(enum engine_builtin_shader_types type){
    return builtin_shaders[type];
}