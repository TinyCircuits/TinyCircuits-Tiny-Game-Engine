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
    mp_obj_base_t base;                 // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;       // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
    mp_obj_t tick_dest[2];              // Used for caching data used for calling the 'tick()' callback on instances of this node
    mp_obj_t position;
    mp_obj_t viewport;
    linked_list_node *camera_list_node; // Used to keep reference into linked list of cameras in the scene
    // TODO: Rotation about x,y,z (matrix?)
}engine_camera_node_class_obj_t;

extern const mp_obj_type_t engine_camera_node_class_type;

// Not static since used by other files to create new camera object
mp_obj_t camera_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // CAMERA_NODE_H