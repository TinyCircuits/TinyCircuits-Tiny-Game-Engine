#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include "py/obj.h"
#include "node_base.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"

// Camera nodes define the transform offset and
// rotation in which everything is rendered.
typedef struct{
    mp_obj_t position;
    mp_obj_t rotation;
    mp_obj_t viewport;
}engine_camera_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    linked_list_node *camera_list_node;
}engine_camera_node_common_data_t;

extern const mp_obj_type_t engine_camera_node_class_type;

// Not static since used by other files to create new camera object
mp_obj_t camera_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // CAMERA_NODE_H