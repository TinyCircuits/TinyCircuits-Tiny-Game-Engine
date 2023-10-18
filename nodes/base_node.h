#ifndef BASE_NODE_H
#define BASE_NODE_H


#include "utility/linked_list.h"


// Class type
typedef struct {
    mp_obj_base_t base;
    mp_obj_t *child;
    linked_list_node *object_list_node;
    mp_obj_t tick_dest[2];
    uint16_t layer;
    bool visible;       // true: drawn and callbacks called, false: not drawn and callbacks called
    bool disabled;      // true: drawn but callbacks not called, false: drawn and callbacks called
    bool just_added;    // When a node is added this flag will be set true. Let's the engine main loop this node was added this game cycle and to not call its callbacks until the next
}engine_base_node_class_obj_t;

const mp_obj_type_t engine_base_node_class_type;


#endif  // BASE_NODE_H