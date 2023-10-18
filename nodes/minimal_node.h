#ifndef MINIMAL_NODE_H
#define MINIMAL_NODE_H

#include "py/obj.h"
#include "node_base.h"

// Minimal node class type. This is used to make generic nodes into
// something with a 'engine_node_base_t node_base' that holds information
// about the type of node. All node instances can be decoded down to this 
// minimal layer for type detection
// Not exposed to MicroPython
typedef struct{
    mp_obj_base_t base;             // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;   // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
}engine_minimal_node_class_obj_t;

#endif  // MINIMAL_NODE_H