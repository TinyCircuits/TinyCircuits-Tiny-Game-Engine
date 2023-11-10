#ifndef EMPTY_NODE_H
#define EMPTY_NODE_H

#include "py/obj.h"
#include "node_base.h"

// A node that doesn't do anything or has a position.
// This can be used in games when there is only a need
// to call code but do nothing else
typedef struct{
    mp_obj_t tick_cb;
}engine_empty_node_common_data_t;

extern const mp_obj_type_t engine_empty_node_class_type;

#endif  // EMPTY_NODE_H