#ifndef NODE_BASE_H
#define NODE_BASE_H

#include "py/obj.h"
#include "utility/bits.h"
#include "utility/linked_list.h"

#define NODE_BASE_VISIBLE_BIT_INDEX 0
#define NODE_BASE_DISABLED_BIT_INDEX 1
#define NODE_BASE_JUST_ADDED_BIT_INDEX 2
#define NODE_BASE_DELETABLE_BIT_INDEX 3

typedef struct{
    mp_obj_base_t base;                     // All nodes get defined by what is placed in this
    linked_list_node *object_list_node;     // Pointer to where this node is stored in the layers of linked lists the engine tracks (used for easy linked list deletion)
    linked_list_node *deletable_list_node;  // Pointer to delete linked list node so that node can remove itself from the list if gc'ed before node clear step
    uint8_t layer;                          // The layer index of the linked list the 'object_list_node' lives in (used for easy deletion)
    uint8_t meta_data;                      // Holds bits related to if this node is visible (not shown or shown but callbacks still called), disabled (callbacks not called but still shown), or just added
    uint8_t type;                           // The type of this node (see 'node_types.h')
    void *attr_accessor;                    // Used in conjunction with mp_get_attr
    void *node;                             // Points to subclass if 'inherited' true otherwise to engine node struct
    
    linked_list children_node_bases;                // Linked list of child node_bases
    void *parent_node_base;                         // If this is a child, pointer to parent node_base (can only have one parent)
    linked_list_node *location_in_parents_children; // The location of this node in the parents linked list of children (used for easy deletion upon garbage collection of this node)
}engine_node_base_t;


// Common data that 2D nodes inherit
typedef struct{
    float px;
    float py;

    float rotation;

    float sx;
    float sy;

    float opacity;
    bool is_camera_child;
}engine_inheritable_2d_t;


void node_base_init(engine_node_base_t *node_base, const mp_obj_type_t *mp_type, uint8_t node_type, uint8_t layer);
bool node_base_is_visible(engine_node_base_t *node_base);
void node_base_set_if_visible(engine_node_base_t *node_base, bool is_visible);
bool node_base_is_disabled(engine_node_base_t *node_base);
void node_base_set_if_disabled(engine_node_base_t *node_base, bool is_disabled);
bool node_base_is_just_added(engine_node_base_t *node_base);
void node_base_set_if_just_added(engine_node_base_t *node_base, bool is_just_added);
bool node_base_is_deletable(engine_node_base_t *node_base);
void node_base_set_if_deletable(engine_node_base_t *node_base, bool is_deletable);


// Given an object that may be a Python class instance or the node_base itself,
// get the node_base from it. Returns `true` if instance and `false` if not
engine_node_base_t *node_base_get(mp_obj_t object, bool *is_obj_instance);

mp_obj_t node_base_del(mp_obj_t self_in);
static MP_DEFINE_CONST_FUN_OBJ_1(node_base_del_obj, node_base_del);

mp_obj_t node_base_mark_destroy(mp_obj_t self_in);
static MP_DEFINE_CONST_FUN_OBJ_1(node_base_mark_destroy_obj, node_base_mark_destroy);

mp_obj_t node_base_mark_destroy_children(mp_obj_t self_in);
static MP_DEFINE_CONST_FUN_OBJ_1(node_base_mark_destroy_children_obj, node_base_mark_destroy_children);

mp_obj_t node_base_mark_destroy_all(mp_obj_t self_in);
static MP_DEFINE_CONST_FUN_OBJ_1(node_base_mark_destroy_all_obj, node_base_mark_destroy_all);

mp_obj_t node_base_add_child(mp_obj_t self_parent_in, mp_obj_t child_in);
static MP_DEFINE_CONST_FUN_OBJ_2(node_base_add_child_obj, node_base_add_child);

mp_obj_t node_base_get_child(mp_obj_t self_parent_in, mp_obj_t index_obj);
static MP_DEFINE_CONST_FUN_OBJ_2(node_base_get_child_obj, node_base_get_child);

mp_obj_t node_base_get_child_count(mp_obj_t self_parent_in);
static MP_DEFINE_CONST_FUN_OBJ_1(node_base_get_child_count_obj, node_base_get_child_count);

mp_obj_t node_base_remove_child(mp_obj_t self_parent_in, mp_obj_t child_in);
static MP_DEFINE_CONST_FUN_OBJ_2(node_base_remove_child_obj, node_base_remove_child);

// Fills 'inheritable' with data from parents and child
void node_base_inherit_2d(mp_obj_t child_node_base, engine_inheritable_2d_t *inheritable);

void node_base_get_child_absolute_xy(float *x, float *y, float *rotation, bool *is_child_of_camera, mp_obj_t child_node_base);

void node_base_set_attr_handler_default(mp_obj_t node_instance);
void node_base_use_default_attr_handler(mp_obj_t self_in, qstr attribute, mp_obj_t *destination);

// Store default Python class instance attr function
// and override with custom intercept attr function
// so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
void node_base_set_attr_handler(mp_obj_t node_instance, mp_attr_fun_t (*default_instance_attr_func)(mp_obj_t self_in, qstr attribute, mp_obj_t *destination));

typedef bool (*attr_handler_func)(engine_node_base_t *node_base, qstr attr, mp_obj_t *dest);

bool node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination);
bool node_base_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination);

// Use this to defer attr lookups through multiple different
// handling functions. Be careful, the order of the functions
// in the passed arrays will mean that if an attribute name
// is found in an early function but was meant to be found
// later (under the same name), then the handling could result
// in hard to diagnose bugs. Check for attr name collisions!
void node_base_attr_handler(mp_obj_t self, qstr attr, mp_obj_t *dest,
                            attr_handler_func load_functions[],
                            attr_handler_func store_functions[],
                            uint8_t attr_function_count);


#endif  // NODE_BASE_H