#ifndef NODE_BASE_H
#define NODE_BASE_H

#include "py/obj.h"
#include "utility/bits.h"
#include "utility/linked_list.h"

#define NODE_BASE_VISIBLE_BIT_INDEX 0
#define NODE_BASE_DISABLED_BIT_INDEX 1
#define NODE_BASE_JUST_ADDED_BIT_INDEX 2

typedef struct{
    mp_obj_base_t base;                 // All nodes get defined by what is placed in this
    linked_list_node *object_list_node; // Pointer to where this node is stored in the layers of linked lists the engine tracks (used for easy linked list deletion)
    bool inherited;                     // Indicator for if this node is part of a Python subclass
    uint16_t layer;                     // The layer index of the linked list the 'object_list_node' lives in (used for easy deletion)
    uint8_t meta_data;                  // Holds bits related to if this node is visible (not shown or shown but callbacks still called), disabled (callbacks not called but still shown), or just added
    uint8_t type;                       // The type of this node (see 'node_types.h')
    void *node;                         // Points to subclass if 'inherited' true otherwise to engine node struct
    void *node_common_data;             // Common data for inherited and non-inherited nodes to always set here
    
    linked_list children_node_bases;                // Linked list of child node_bases
    void *parent_node_base;                         // If this is a child, pointer to parent node_base (can only have one parent)
    linked_list_node *location_in_parents_children; // The location of this node in the parents linked list of children (used for easy deletion upon garbage collection of this node)
}engine_node_base_t;

mp_obj_t node_base_del(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(node_base_del_obj, node_base_del);

mp_obj_t node_base_set_layer(mp_obj_t self_in, mp_obj_t layer);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(node_base_set_layer_obj, node_base_set_layer);

mp_obj_t node_base_get_layer(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(node_base_get_layer_obj, node_base_get_layer);

mp_obj_t node_base_add_child(mp_obj_t self_parent_in, mp_obj_t child_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(node_base_add_child_obj, node_base_add_child);


inline bool node_base_is_visible(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
}


inline void node_base_set_if_visible(engine_node_base_t *node_base, bool is_visible){
    if(is_visible){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
    }
}


inline bool node_base_is_disabled(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
}

inline void node_base_set_if_disabled(engine_node_base_t *node_base, bool is_disabled){
    if(is_disabled){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
    }
}


inline bool node_base_is_just_added(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
}

inline void node_base_set_if_just_added(engine_node_base_t *node_base, bool is_just_added){
    if(is_just_added){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
    }
}


#endif  // NODE_BASE_H