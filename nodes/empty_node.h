#ifndef EMPTY_NODE_H
#define EMPTY_NODE_H

#include "py/obj.h"
#include "node_base.h"

// A node that doesn't do anything or has a position.
// This can be used in games when there is only a need
// to call code but do nothing else
typedef struct{
    mp_obj_base_t base;             // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;   // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
    mp_obj_t tick_dest[2];          // Used for caching data used for calling the 'tick()' callback on instances of this node
}engine_empty_node_class_obj_t;

const mp_obj_type_t engine_empty_node_class_type;

#endif  // EMPTY_NODE_H