#ifndef EMPTY_NODE_H
#define EMPTY_NODE_H

#include "py/obj.h"

// A node that doesn't do anything or has a position.
// This can be used in games when there is only a need
// to call code but do nothing else
typedef struct{
    mp_obj_t position;              // Vector3: xyz position of this node
    mp_obj_t rotation;              // Vector3: rotation of this node in space
    mp_obj_t tick_cb;
}engine_empty_node_class_obj_t;

extern const mp_obj_type_t engine_empty_node_class_type;

#endif  // EMPTY_NODE_H