#ifndef SPRITE_2D_NODE_H
#define SPRITE_2D_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// A basic 2d sprite node
typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t texture_resource;      // TextureResource
    mp_obj_t transparent_color;     // 16-bit integer representing which exact color in the BMP to not render
    mp_obj_t fps;                   // How many frames per second the sprite should play its animation (if possible)
    mp_obj_t frame_count_x;
    mp_obj_t frame_count_y;
    mp_obj_t frame_current_x;
    mp_obj_t frame_current_y;
    mp_obj_t rotation;              // Rotation about into screen/z-axis in degrees
    mp_obj_t scale;                 // Vector2
    mp_obj_t opacity;
    mp_obj_t playing;               // Bool: is the animation running or not
    mp_obj_t loop;
    mp_obj_t tick_cb;
    uint32_t time_at_last_animation_update_ms;
}engine_sprite_2d_node_class_obj_t;

extern const mp_obj_type_t engine_sprite_2d_node_class_type;
void sprite_2d_node_class_draw(mp_obj_t sprite_node_base_obj, mp_obj_t camera_node);

#endif  // SPRITE_2D_NODE_H