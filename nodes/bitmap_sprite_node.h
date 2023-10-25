#ifndef BITMAP_SPRITE_NODE_H
#define BITMAP_SPRITE_NODE_H

#include "py/obj.h"
#include "node_base.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"

// A basic bitmap sprite node with basic
typedef struct{
    mp_obj_base_t base;             // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;   // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
    mp_obj_t tick_dest[2];          // Used for caching data used for calling the 'tick()' callback on instances of this node
    mp_obj_t draw_dest[2];          // Used for caching data used for calling the 'draw()' callback on instances of this node
    mp_obj_t position;
    uint16_t *bitmap_data;          // 16-bit pixel data of the bitmap (can be frames)
    mp_uint_t width;                // Bitmap width in px
    mp_uint_t height;               // Bitmap height in px
    mp_uint_t frame_count;          // Number of frames in the bitmap node (animation)
    mp_float_t animation_fps;       // How many frames per second the sprite should play its animation (if possible)
    mp_float_t rotation;            // Rotation in degrees
    mp_float_t scale;               // Scale in percent %
}engine_bitmap_sprite_node_class_obj_t;

extern const mp_obj_type_t engine_bitmap_sprite_node_class_type;

#endif  // BITMAP_SPRITE_NODE_H