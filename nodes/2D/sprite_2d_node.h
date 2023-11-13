#ifndef SPRITE_2D_NODE_H
#define SPRITE_2D_NODE_H

#include "py/obj.h"

// A basic 2d sprite node
typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t width;                 // Rectangle width in px
    mp_obj_t height;                // Rectangle height in px
    mp_obj_t filename;              // The filename of the sprite data in flash/LittleFS
    mp_obj_t fps;                   // How many frames per second the sprite should play its animation (if possible)
    mp_obj_t rotation;              // Rotation about into screen/z-axis in degrees
    mp_obj_t scale;                 // Vector2
}engine_sprite_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_sprite_2d_node_common_data_t;

extern const mp_obj_type_t engine_sprite_2d_node_class_type;

#endif  // SPRITE_2D_NODE_H