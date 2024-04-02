#ifndef BUTTON_2D_NODE_H
#define BUTTON_2D_NODE_H

#include "py/obj.h"
#include "math/vector2.h"
#include "nodes/node_base.h"

typedef struct{
    mp_obj_t position;
    mp_obj_t font_resource;
    mp_obj_t text;
    mp_obj_t rotation;
    mp_obj_t scale;
    mp_obj_t opacity;
    mp_obj_t tick_cb;
    bool focused;
    float width;
    float height;
}engine_button_2d_node_class_obj_t;

extern const mp_obj_type_t engine_button_2d_node_class_type;
void button_2d_node_class_draw(engine_node_base_t *button_node_base, mp_obj_t camera_node);

#endif  // BUTTON_2D_NODE_H