#ifndef MESH_NODE_H
#define MESH_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

typedef struct{
    mp_obj_t position;  // Vector3
    mp_obj_list_t *vertices;
    mp_obj_t tick_cb;
}engine_mesh_node_class_obj_t;

extern const mp_obj_type_t engine_mesh_node_class_type;
void mesh_node_class_draw(mp_obj_t mesh_node_base_obj, mp_obj_t camera_node);

#endif  // MESH_NODE_H