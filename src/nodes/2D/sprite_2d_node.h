#ifndef SPRITE_2D_NODE_H
#define SPRITE_2D_NODE_H

#include "py/obj.h"

// A basic 2d sprite node
typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t fps;                   // How many frames per second the sprite should play its animation (if possible)
    mp_obj_t rotation;              // Rotation about into screen/z-axis in degrees
    mp_obj_t scale;                 // Vector2
    mp_obj_t texture_resource;      // TextureResource
    mp_obj_t transparent_color;     // 16-bit integer representing which exact color in the BMP to not render
}engine_sprite_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_sprite_2d_node_common_data_t;

extern const mp_obj_type_t engine_sprite_2d_node_class_type;

#endif  // SPRITE_2D_NODE_H