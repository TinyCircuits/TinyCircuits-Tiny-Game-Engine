#ifndef NODE_BASE_H
#define NODE_BASE_H

#include "py/obj.h"
#include "utility/bits.h"
#include "utility/linked_list.h"

#define NODE_BASE_VISIBLE_BIT_INDEX 0
#define NODE_BASE_DISABLED_BIT_INDEX 1
#define NODE_BASE_JUST_ADDED_BIT_INDEX 2

typedef struct{
    mp_obj_base_t base;
    uint8_t type;                       // The type of this node (see 'node_types.h')
    linked_list_node *object_list_node; // Pointer to where this node is stored in the layers of linked lists the engine tracks (used for easy linked list deletion)
    uint16_t layer;                     // The layer index of the linked list the 'object_list_node' lives in (used for easy deletion)
    uint8_t meta_data;                  // Holds bits related to if this node is visible (not shown or shown but callbacks still called), disabled (callbacks not called but still shown), or just added
    mp_obj_t tick_cb;                   // Used for caching data used for calling the 'tick()' callback on instances of this node
    mp_obj_t draw_cb;                   // Used for caching data used for calling the 'draw()' callback on instances of this node
    void *node;
}engine_node_base_t;


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