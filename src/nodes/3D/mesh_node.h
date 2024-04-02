#ifndef MESH_NODE_H
#define MESH_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// A basic 2d circle node
typedef struct{
    mp_obj_t position;  // Vector3
    mp_obj_list_t *vertices;
    mp_obj_t tick_cb;
}engine_mesh_node_class_obj_t;

extern const mp_obj_type_t engine_mesh_node_class_type;
void mesh_node_class_draw(engine_node_base_t *mesh_node_base, mp_obj_t camera_node);

#endif  // MESH_NODE_H