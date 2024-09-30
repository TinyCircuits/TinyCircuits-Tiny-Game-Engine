#ifndef MESH_NODE_H
#define MESH_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

#define CGLM_CLIPSPACE_INCLUDE_ALL 1
#include "../lib/cglm/include/cglm/cglm.h"
#include "../lib/cglm/include/cglm/vec3.h"
#include "../lib/cglm/include/cglm/mat4.h"
#include "../lib/cglm/include/cglm/cam.h"
#include "../lib/cglm/include/cglm/euler.h"


typedef struct{
    mp_obj_t position;  // Vector3
    mp_obj_t rotation;  // Vector3
    mp_obj_t scale;     // Vector3
    mp_obj_t mesh;      // MeshResource
    mp_obj_t color;
    mp_obj_t tick_cb;

    mat4 m_translation;
    mat4 m_rotation;
    mat4 m_scale;
}engine_mesh_node_class_obj_t;

extern const mp_obj_type_t engine_mesh_node_class_type;
void mesh_node_class_draw(mp_obj_t mesh_node_base_obj, mp_obj_t camera_node);

#endif  // MESH_NODE_H