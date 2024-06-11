#include "py/objtype.h"

#include "node_base.h"
#include "engine_object_layers.h"
#include "math/engine_math.h"
#include "node_types.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "utility/engine_mp.h"
#include "py/misc.h"
#include "engine_collections.h"


/*  --- doc ---
    NAME: tick
    ID: tick
    DESC: Overridable tick callback
    PARAM: [type=float] [name=dt] [value=positive float in seconds]                                                                                                  
    RETURN: None
*/ 
/*  --- doc ---
    NAME: draw
    ID: draw
    DESC: Overridable draw callback 
    PARAM: [type=Node] [name=camera] [value=one of the camera nodes in the scene]                                                                                                         
    RETURN: None
*/ 
void (*default_instance_attr_func)(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) = NULL;


void node_base_init(engine_node_base_t *node_base, const mp_obj_type_t *mp_type, uint8_t node_type){
    node_base->base.type = mp_type;
    node_base->layer = 0;
    node_base->type = node_type;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base->parent_node_base = NULL;
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);
}


bool node_base_is_visible(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
}


void node_base_set_if_visible(engine_node_base_t *node_base, bool is_visible){
    if(is_visible){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_VISIBLE_BIT_INDEX);
    }
}


bool node_base_is_disabled(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
}

void node_base_set_if_disabled(engine_node_base_t *node_base, bool is_disabled){
    if(is_disabled){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_DISABLED_BIT_INDEX);
    }
}


bool node_base_is_just_added(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
}

void node_base_set_if_just_added(engine_node_base_t *node_base, bool is_just_added){
    if(is_just_added){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_JUST_ADDED_BIT_INDEX);
    }
}


bool node_base_is_deletable(engine_node_base_t *node_base){
    return BIT_GET(node_base->meta_data, NODE_BASE_DELETABLE_BIT_INDEX);
}

void node_base_set_if_deletable(engine_node_base_t *node_base, bool is_deletable){
    if(is_deletable){
        BIT_SET_TRUE(node_base->meta_data, NODE_BASE_DELETABLE_BIT_INDEX);
    }else{
        BIT_SET_FALSE(node_base->meta_data, NODE_BASE_DELETABLE_BIT_INDEX);
    }
}


engine_node_base_t *node_base_get(mp_obj_t object, bool *is_obj_instance){
    bool is_instance = mp_obj_is_instance_type(((mp_obj_base_t*)object)->type);
    engine_node_base_t *to_return = NULL;

    if(is_instance){
        mp_obj_t dest[2];
        dest[0] = MP_OBJ_NULL; // Indicate we want to load a value
        node_base_use_default_attr_handler(object, MP_QSTR_node_base, dest);
        to_return = dest[0];
    }else{
        to_return = object;
    }

    if(is_obj_instance != NULL) *is_obj_instance = is_instance;
    return to_return;
}


mp_obj_t node_base_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Node Base: Deleted (garbage collected, removing self from active engine objects): %s", mp_obj_get_type_str(self_in));

    engine_node_base_t *node_base = self_in;

    // If this is a child of another node, remove this as a child
    if(node_base->parent_node_base != NULL){
        engine_node_base_t *parent_node_base = node_base->parent_node_base;

        linked_list_del_list_node(&parent_node_base->children_node_bases, node_base->location_in_parents_children);

        node_base->location_in_parents_children = NULL;
        node_base->parent_node_base = NULL;
    }

    // If this node has child nodes, go through all children and unlink to this as a parent
    if(node_base->children_node_bases.count != 0){
        linked_list_node *current_child_node_base = node_base->children_node_bases.start;

        while(current_child_node_base != NULL){
            engine_node_base_t *child_node_base = current_child_node_base->object;
            child_node_base->parent_node_base = NULL;
            child_node_base->location_in_parents_children = NULL;

            linked_list_node *next_child_node_base = current_child_node_base->next;

            // This linked list is being deleted, free each node as we go
            free(current_child_node_base);

            current_child_node_base = next_child_node_base;
        }
    }

    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);

    return mp_const_none;
}


/*  --- doc ---
    NAME: destroy
    ID: node_base_destroy
    DESC: Destroys node. Calls finalizer and frees memory for MicroPython to use later                                                                         
    RETURN: None
*/ 
mp_obj_t node_base_destroy(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;

    // Don't want to track a node for deletion twice
    if(node_base_is_deletable(node_base) == false){
        node_base_set_if_deletable(node_base, true);
        engine_collections_track_deletable(node_base);
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: destroy_children
    ID: node_base_destroy_children
    DESC: Destroys only the children of this node as well as the childrens' children. Calls finalizer and frees memory for MicroPython to use later                                                                         
    RETURN: None
*/ 
mp_obj_t node_base_destroy_children(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;

    // If no children, don't do anything more
    if(node_base->children_node_bases.count == 0){
        return mp_const_none;
    }

    // If this node has child nodes, go through all children and destroy them
    linked_list_node *current_child_link_node = node_base->children_node_bases.start;
    while(current_child_link_node != NULL){
        engine_node_base_t *child_node_base = current_child_link_node->object;
        node_base_destroy_children(child_node_base);
        node_base_destroy(child_node_base);
        current_child_link_node = node_base->children_node_bases.start;
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: destroy_all
    ID: node_base_destroy_all
    DESC: Destroys node, its children, and the childrens' children. Calls finalizer and frees memory for MicroPython to use later for each node                                                                                              
    RETURN: None
*/ 
mp_obj_t node_base_destroy_all(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;
    node_base_destroy_children(self_in);
    node_base_destroy(node_base);

    return mp_const_none;
}


/*  --- doc ---
    NAME: add_child
    ID: add_child
    DESC: Adds child to the node this is being called on
    PARAM: [type=Node] [name=child] [value=any node]                                                                                                         
    RETURN: None
*/ 
mp_obj_t node_base_add_child(mp_obj_t self_parent_in, mp_obj_t child_in){
    // Get the node_base for cases with the child is a Python
    // class instance or just the node's native built-in type
    // without inheritance
    engine_node_base_t *parent_node_base = node_base_get(self_parent_in, NULL);
    engine_node_base_t *child_node_base = node_base_get(child_in, NULL);
    
    ENGINE_INFO_PRINTF("Node Base: Adding child... parent node type: %d, child node type: %d", parent_node_base->type, child_node_base->type);

    child_node_base->location_in_parents_children = linked_list_add_obj(&parent_node_base->children_node_bases, child_node_base);
    child_node_base->parent_node_base = parent_node_base;

    return mp_const_none;
}


/*  --- doc ---
    NAME: get_child
    ID: get_child
    DESC: Gets child
    PARAM: [type=int] [name=index] [value=any positive integer]                                                                                                        
    RETURN: Node
*/ 
mp_obj_t node_base_get_child(mp_obj_t self_parent_in, mp_obj_t index_obj){
    ENGINE_INFO_PRINTF("Node Base: Getting child...");

    engine_node_base_t *parent_node_base = self_parent_in;
    uint16_t child_index = mp_obj_get_int(index_obj);

    uint16_t current_index = 0;
    linked_list_node *current_child_node = parent_node_base->children_node_bases.start;
    while(current_index < child_index){
        current_index++;

        if(current_child_node->next == NULL){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Tried to access a child node out of bounds!"));
        }

        current_child_node = current_child_node->next;
    }

    return current_child_node->object;
}


/*  --- doc ---
    NAME: get_child_count
    ID: get_child_count
    DESC: Gets the count of children directly descended from this node but not the children of the children                                                                                                       
    RETURN: 0 or positive integer
*/ 
mp_obj_t node_base_get_child_count(mp_obj_t self_parent_in){
    ENGINE_INFO_PRINTF("Node Base: Getting child count...");

    engine_node_base_t *parent_node_base = self_parent_in;
    return mp_obj_new_int(parent_node_base->children_node_bases.count);
}


/*  --- doc ---
    NAME: remove_child
    ID: remove_child
    DESC: Removes child from the node this is being called on
    PARAM: [type=Node] [name=child] [value=any node]                                                                                                        
    RETURN: None
*/ 
mp_obj_t node_base_remove_child(mp_obj_t self_parent_in, mp_obj_t child_in){
    ENGINE_INFO_PRINTF("Node Base: Removing child...");

    engine_node_base_t *parent_node_base = self_parent_in;
    engine_node_base_t *child_node_base = child_in;

    // Check if the child's parent equals this node's memory location (bad idea?)
    if(child_node_base->parent_node_base == parent_node_base){
        linked_list_del_list_node(&parent_node_base->children_node_bases, child_node_base->location_in_parents_children);
        child_node_base->parent_node_base = NULL;
        child_node_base->location_in_parents_children = NULL;
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Child node does not exist on this parent, cannot remove!"));
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: set_layer
    ID: set_layer
    DESC: Sets the node to a different render layer. Draws layer 0 first and up to 7 last.
    PARAM: [type=int] [name=layer] [value=0 ~ 7]                                                                                                        
    RETURN: None
*/ 
mp_obj_t node_base_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Node Base: Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}


/*  --- doc ---
    NAME: get_layer
    ID: get_layer
    DESC: Gets the render layer the node is currently on                                                                                                     
    RETURN: 0 ~ 7
*/ 
mp_obj_t node_base_get_layer(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Node Base: Getting object layer...");

    engine_node_base_t *node_base = self_in;
    return mp_obj_new_int(node_base->layer);
}


void node_base_get_child_absolute_xy(float *x, float *y, float *rotation, bool *is_child_of_camera, mp_obj_t child_node_base_in){
    engine_node_base_t *child_node_base = child_node_base_in;
    if(is_child_of_camera != NULL) *is_child_of_camera = false;

    mp_obj_t child_node_base_position_obj = mp_load_attr(child_node_base->attr_accessor, MP_QSTR_position);
    if(mp_obj_is_type(child_node_base_position_obj, &vector3_class_type)){
        *x = ((vector3_class_obj_t*)child_node_base_position_obj)->x.value;
        *y = ((vector3_class_obj_t*)child_node_base_position_obj)->y.value;
    }else{
        *x = ((vector2_class_obj_t*)child_node_base_position_obj)->x.value;
        *y = ((vector2_class_obj_t*)child_node_base_position_obj)->y.value;
    }

    mp_obj_t rotation_obj = engine_mp_load_attr_maybe(child_node_base->attr_accessor, MP_QSTR_rotation);

    // Use z-axis rotation for 2D rotations from 3D vectors
    if(rotation_obj == MP_OBJ_NULL){
        // In the case that the rotation attribute does not exist on this node, set rotation to 0
        if(rotation != NULL) *rotation = 0.0f;
    }else if(mp_obj_is_type(rotation_obj, &vector3_class_type)){
        if(rotation != NULL) *rotation = ((vector3_class_obj_t*)rotation_obj)->z.value;
    }else{
        if(rotation != NULL) *rotation = (float)mp_obj_get_float(rotation_obj);
    }

    // Before doing anything, check if this child even has a parent 
    if(child_node_base->parent_node_base != NULL){
        engine_node_base_t *seeking_node_base = child_node_base;
        
        while(true){
            engine_node_base_t *seeking_parent_node_base = seeking_node_base->parent_node_base;

            if(seeking_parent_node_base != NULL){
                // Need to know if a child of a camera so that 
                // certain scaling and translations do not occur
                if(is_child_of_camera != NULL && seeking_parent_node_base->type == NODE_TYPE_CAMERA){
                    *is_child_of_camera = true;
                }

                mp_obj_t parent_position_obj = mp_load_attr(seeking_parent_node_base->attr_accessor, MP_QSTR_position);
                mp_obj_t parent_rotation_obj = engine_mp_load_attr_maybe(seeking_parent_node_base->attr_accessor, MP_QSTR_rotation);

                float parent_x = 0.0f;
                float parent_y = 0.0f;
                float parent_rotation_radians = 0.0f;

                if(mp_obj_is_type(parent_position_obj, &vector3_class_type)){
                    parent_x = ((vector3_class_obj_t*)parent_position_obj)->x.value;
                    parent_y = ((vector3_class_obj_t*)parent_position_obj)->y.value;
                }else{
                    parent_x = ((vector2_class_obj_t*)parent_position_obj)->x.value;
                    parent_y = ((vector2_class_obj_t*)parent_position_obj)->y.value;
                }

                if(parent_rotation_obj == MP_OBJ_NULL){
                    // In the case that the rotation attribute does not exist on this node, set rotation to 0
                    parent_rotation_radians = 0.0f;
                }else if(mp_obj_is_type(parent_rotation_obj, &vector3_class_type)){
                    parent_rotation_radians = ((vector3_class_obj_t*)parent_rotation_obj)->z.value;
                }else{
                    parent_rotation_radians = (float)mp_obj_get_float(parent_rotation_obj);
                }

                if(is_child_of_camera == NULL || (is_child_of_camera != NULL && *is_child_of_camera == false)){
                    *x += parent_x;
                    *y += parent_y;
                    if(rotation != NULL) *rotation += parent_rotation_radians;
                    
                    // If the rotation sum is not close to zero, rotate the point (small optimization)
                    if(engine_math_compare_floats(*rotation, 0.0f) == false){
                        engine_math_rotate_point(x, y, parent_x, parent_y, parent_rotation_radians);
                    }
                }
            }else{
                // Done, reached top-most parent
                break;
            }

            seeking_node_base = seeking_node_base->parent_node_base;
        }
    }
}


void node_base_set_attr_handler_default(mp_obj_t node_instance){
    if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);
}


void node_base_set_attr_handler(mp_obj_t node_instance, mp_attr_fun_t (*attr_handler_func)(mp_obj_t self_in, qstr attribute, mp_obj_t *destination)){
    default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
    MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, attr_handler_func, 5);
}


void node_base_use_default_attr_handler(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    default_instance_attr_func(self_in, attribute, destination);
}