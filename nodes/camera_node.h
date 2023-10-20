#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include "py/obj.h"
#include "node_base.h"
#include "math/vector3.h"

// Camera nodes define the transform offset and
// rotation in which everything is rendered.
typedef struct{
    mp_obj_base_t base;             // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;   // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
    mp_obj_t tick_dest[2];          // Used for caching data used for calling the 'tick()' callback on instances of this node
    mp_obj_t position;
    // TODO: Rotation about x,y,z (matrix?)
}engine_camera_node_class_obj_t;

const mp_obj_type_t engine_camera_node_class_type;

#endif  // CAMERA_NODE_H