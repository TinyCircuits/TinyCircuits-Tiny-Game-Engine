#ifndef NODE_BASE_H
#define NODE_BASE_H

#include "py/obj.h"
#include "utility/bits.h"
#include "utility/linked_list.h"

#define NODE_BASE_VISIBLE_BIT_INDEX 0
#define NODE_BASE_DISABLED_BIT_INDEX 1
#define NODE_BASE_JUST_ADDED_BIT_INDEX 2

// Because the attr function for classes defined in Python
// is different than builtin types, we store a reference
// to the default function and call it through an intercept
// function. The function that is being stored here is
// https://github.com/micropython/micropython/blob/0432f73206a0d0ff5cef5dc6146f18439079f6f3/py/objtype.c#L795-L803
//
// https://github.com/orgs/micropython/discussions/13109
// https://forum.micropython.org/viewtopic.php?t=1044
// https://github.com/micropython/micropython/wiki/Differences#implementation-differences
// https://github.com/micropython/micropython/issues/401
// https://forum.micropython.org/viewtopic.php?f=2&t=7028&start=10
void (*default_instance_attr_func)(mp_obj_t self_in, qstr attribute, mp_obj_t *destination);

typedef struct{
    mp_obj_base_t base;                 // All nodes get defined by what is placed in this
    linked_list_node *object_list_node; // Pointer to where this node is stored in the layers of linked lists the engine tracks (used for easy linked list deletion)
    uint16_t layer;                     // The layer index of the linked list the 'object_list_node' lives in (used for easy deletion)
    uint8_t meta_data;                  // Holds bits related to if this node is visible (not shown or shown but callbacks still called), disabled (callbacks not called but still shown), or just added
    uint8_t type;                       // The type of this node (see 'node_types.h')
    void *attr_accessor;                // Used in conjunction with mp_get_attr
    void *node;                         // Points to subclass if 'inherited' true otherwise to engine node struct
    
    linked_list children_node_bases;                // Linked list of child node_bases
    void *parent_node_base;                         // If this is a child, pointer to parent node_base (can only have one parent)
    linked_list_node *location_in_parents_children; // The location of this node in the parents linked list of children (used for easy deletion upon garbage collection of this node)
}engine_node_base_t;


// Given an object that may be a Python class instance or the node_base itself,
// get the node_base from it. Returns `true` if instance and `false` if not
engine_node_base_t *node_base_get(mp_obj_t object, bool *is_obj_instance);

mp_obj_t node_base_del(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(node_base_del_obj, node_base_del);

mp_obj_t node_base_add_child(mp_obj_t self_parent_in, mp_obj_t child_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(node_base_add_child_obj, node_base_add_child);

mp_obj_t node_base_get_child(mp_obj_t self_parent_in, mp_obj_t index_obj);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(node_base_get_child_obj, node_base_get_child);

mp_obj_t node_base_remove_child(mp_obj_t self_parent_in, mp_obj_t child_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(node_base_remove_child_obj, node_base_remove_child);

mp_obj_t node_base_set_layer(mp_obj_t self_in, mp_obj_t layer);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(node_base_set_layer_obj, node_base_set_layer);

mp_obj_t node_base_get_layer(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(node_base_get_layer_obj, node_base_get_layer);


void node_base_get_child_absolute_xy(float *x, float *y, float *rotation, bool *is_child_of_camera, mp_obj_t child_node_base);

void node_base_init(engine_node_base_t *node_base, const mp_obj_type_t *mp_type, uint8_t node_type);
bool node_base_is_visible(engine_node_base_t *node_base);
void node_base_set_if_visible(engine_node_base_t *node_base, bool is_visible);
bool node_base_is_disabled(engine_node_base_t *node_base);
void node_base_set_if_disabled(engine_node_base_t *node_base, bool is_disabled);
bool node_base_is_just_added(engine_node_base_t *node_base);
void node_base_set_if_just_added(engine_node_base_t *node_base, bool is_just_added);


#endif  // NODE_BASE_H