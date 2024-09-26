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
    PARAM: [type=object] [name=self] [value=object]
    PARAM: [type=float]  [name=dt] [value=positive float in seconds]                                                                                                  
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


void node_base_init(engine_node_base_t *node_base, const mp_obj_type_t *mp_type, uint8_t node_type, uint8_t layer){
    node_base->base.type = mp_type;
    node_base->layer = layer;
    node_base->type = node_type;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base->deletable_list_node = NULL;
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
    engine_collections_untrack_deletable(node_base->deletable_list_node);

    return mp_const_none;
}


/*  --- doc ---
    NAME: mark_destroy
    ID: node_base_mark_destroy
    DESC: Destroys node. Calls finalizer and frees memory for MicroPython to use later                                                                         
    RETURN: None
*/ 
mp_obj_t node_base_mark_destroy(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;

    // Don't want to track a node for deletion twice
    if(node_base_is_deletable(node_base) == false){
        node_base_set_if_deletable(node_base, true);
        node_base->deletable_list_node = engine_collections_track_deletable(node_base);
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: mark_destroy_children
    ID: node_base_mark_destroy_children
    DESC: Destroys only the children of this node as well as the childrens' children. Calls finalizer and frees memory for MicroPython to use later                                                                         
    RETURN: None
*/ 
mp_obj_t node_base_mark_destroy_children(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;

    // If no children, don't do anything more
    if(node_base->children_node_bases.count == 0){
        return mp_const_none;
    }

    // If this node has child nodes, go through all children and destroy them
    linked_list_node *current_child_link_node = node_base->children_node_bases.start;
    while(current_child_link_node != NULL){
        engine_node_base_t *child_node_base = current_child_link_node->object;
        node_base_mark_destroy_children(child_node_base);
        node_base_mark_destroy(child_node_base);
        current_child_link_node = current_child_link_node->next;
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: mark_destroy_all
    ID: node_base_mark_destroy_all
    DESC: Destroys node, its children, and the childrens' children. Calls finalizer and frees memory for MicroPython to use later for each node                                                                                              
    RETURN: None
*/ 
mp_obj_t node_base_mark_destroy_all(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;
    node_base_mark_destroy_children(self_in);
    node_base_mark_destroy(node_base);

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

    while(true){
        if(current_child_node == NULL){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Child node doesn't exist at that index!"));
        }

        if(current_index == child_index){
            return current_child_node->object;
        }

        current_index++;
        current_child_node = current_child_node->next;
    }

    return mp_const_none;
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


void node_base_set_layer(engine_node_base_t *node_base, uint8_t layer){
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = layer;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
}


void node_base_inherit_2d(mp_obj_t child_node_base, engine_inheritable_2d_t *inheritable){
    engine_node_base_t *node_base = child_node_base;

    // Init structure
    mp_obj_t child_position = mp_load_attr(node_base->attr_accessor, MP_QSTR_position);
    mp_obj_t child_rotation = engine_mp_load_attr_maybe(node_base->attr_accessor, MP_QSTR_rotation);
    mp_obj_t child_scale =    engine_mp_load_attr_maybe(node_base->attr_accessor, MP_QSTR_scale);

    // Setup initial position (no nodes have 1D position)
    if(mp_obj_is_type(child_position, &vector3_class_type)){
        inheritable->px = ((vector3_class_obj_t*)child_position)->x.value;
        inheritable->py = ((vector3_class_obj_t*)child_position)->y.value;
    }else if(mp_obj_is_type(child_position, &vector2_class_type)){
        inheritable->px = ((vector2_class_obj_t*)child_position)->x.value;
        inheritable->py = ((vector2_class_obj_t*)child_position)->y.value;
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to set initial inherit 2D position for this `position `object type!"));
    }

    // Setup initial rotation (no nodes have 2D rotation)
    if(child_rotation == MP_OBJ_NULL){
        inheritable->rotation = 0.0f;
    }else if(mp_obj_is_type(child_rotation, &vector3_class_type)){
        inheritable->rotation = ((vector3_class_obj_t*)child_rotation)->z.value;
    }else if(mp_obj_is_float(child_rotation)){
        inheritable->rotation = (float)mp_obj_get_float(child_rotation);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to set initial inherit 2D rotation for this `rotation `object type!"));
    }

    // Setup initial scale (some nodes, like circles, have 1D scale)
    if(child_scale == MP_OBJ_NULL){
        inheritable->sx = 1.0f;
        inheritable->sy = 1.0f;
    }else if(mp_obj_is_type(child_scale, &vector3_class_type)){
        inheritable->sx = ((vector3_class_obj_t*)child_scale)->x.value;
        inheritable->sy = ((vector3_class_obj_t*)child_scale)->y.value;
    }else if(mp_obj_is_type(child_scale, &vector2_class_type)){
        inheritable->sx = ((vector2_class_obj_t*)child_scale)->x.value;
        inheritable->sy = ((vector2_class_obj_t*)child_scale)->y.value;
    }else if(mp_obj_is_float(child_scale)){
        float scale = mp_obj_get_float(child_scale);
        inheritable->sx = scale;
        inheritable->sy = scale;
    }else if(mp_obj_is_int(child_scale)){
        float scale = (float)mp_obj_get_int(child_scale);
        inheritable->sx = scale;
        inheritable->sy = scale;
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to set initial inherit 2D scale for this `scale `object type!"));
    }

    // Setup initial opacity and `is camera child`
    // flag (most nodes use single float for opacity,
    // except for things like physics and voxelspace)
    mp_obj_t child_opacity_obj = engine_mp_load_attr_maybe(node_base->attr_accessor, MP_QSTR_opacity);
    inheritable->opacity = 1.0f;
    if(child_opacity_obj != MP_OBJ_NULL){
        inheritable->opacity = mp_obj_get_float(child_opacity_obj);
    }
    
    inheritable->is_camera_child = false;

    // Before doing anything else, check if this child
    // even has a parent (still needed to ensure struct
    // is initialzed)
    if(node_base->parent_node_base == NULL){
        return;
    }

    // Start the traversal upwards until we get to a NULL parent
    engine_node_base_t *current_node_base = node_base;

    while(true){
        engine_node_base_t *parent_node_base = current_node_base->parent_node_base;

        // Stop traversal only if the parent of
        // the current traversal node is NULL
        if(parent_node_base == NULL){
            break;
        }

        // Get this to stop translations for camera
        // children due to camera transformation
        if(parent_node_base->type == NODE_TYPE_CAMERA){
            inheritable->is_camera_child = true;

            // Do not care about the rest of the
            // transformations the camera might
            // have due to its parents
            break;
        }

        // Get parent attributes that will affect child
        mp_obj_t parent_position = mp_load_attr(parent_node_base->attr_accessor, MP_QSTR_position);
        mp_obj_t parent_rotation = engine_mp_load_attr_maybe(parent_node_base->attr_accessor, MP_QSTR_rotation);
        mp_obj_t parent_scale = engine_mp_load_attr_maybe(parent_node_base->attr_accessor, MP_QSTR_scale);
        mp_obj_t parent_opacity_obj = engine_mp_load_attr_maybe(parent_node_base->attr_accessor, MP_QSTR_opacity);
        float parent_opacity = 1.0f;
        
        if(parent_opacity_obj != MP_OBJ_NULL){
            parent_opacity = mp_obj_get_float(parent_opacity_obj);
        }

        // Setup temps that are re-used in multiple ways
        float temp_parent_pos_x = 0.0f;
        float temp_parent_pos_y = 0.0f;
        float temp_parent_rot = 0.0f;
        float temp_parent_scale_x = 0.0f;
        float temp_parent_scale_y = 0.0f;

        // Decode parent position (use projection of 3D position
        // for respective 2D position that child will rotate around)
        if(mp_obj_is_type(parent_position, &vector3_class_type)){
            temp_parent_pos_x = ((vector3_class_obj_t*)parent_position)->x.value;
            temp_parent_pos_y = ((vector3_class_obj_t*)parent_position)->y.value;
        }else if(mp_obj_is_type(parent_position, &vector2_class_type)){
            temp_parent_pos_x = ((vector2_class_obj_t*)parent_position)->x.value;
            temp_parent_pos_y = ((vector2_class_obj_t*)parent_position)->y.value;
        }else{
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to get inherited 2D position for this `position `object type!"));
        }

        // Decode parent rotation (use z-axis rotation of 3D node
        // for rotation around parent for 2D child node)
        if(parent_rotation == MP_OBJ_NULL){
            temp_parent_rot = 0.0f;
        }else if(mp_obj_is_type(parent_rotation, &vector3_class_type)){
            temp_parent_rot = ((vector3_class_obj_t*)parent_rotation)->z.value;
        }else if(mp_obj_is_float(parent_rotation)){
            temp_parent_rot = mp_obj_get_float(parent_rotation);
        }else{
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to get inherited 2D rotation for this `rotation `object type!"));
        }

        // Decode parent scale (use xy of 3D for 2D scale)
        if(parent_scale == MP_OBJ_NULL){
            temp_parent_scale_x = 1.0f;
            temp_parent_scale_y = 1.0f;
        }else if(mp_obj_is_type(parent_scale, &vector3_class_type)){
            temp_parent_scale_x = ((vector3_class_obj_t*)parent_scale)->x.value;
            temp_parent_scale_y = ((vector3_class_obj_t*)parent_scale)->y.value;
        }else if(mp_obj_is_type(parent_scale, &vector2_class_type)){
            temp_parent_scale_x = ((vector2_class_obj_t*)parent_scale)->x.value;
            temp_parent_scale_y = ((vector2_class_obj_t*)parent_scale)->y.value;
        }else if(mp_obj_is_float(parent_scale)){
            float scale = mp_obj_get_float(parent_scale);
            temp_parent_scale_x = scale;
            temp_parent_scale_y = scale;
        }else if(mp_obj_is_int(parent_scale)){
            float scale = (float)mp_obj_get_int(parent_scale);
            temp_parent_scale_x = scale;
            temp_parent_scale_y = scale;
        }else{
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: Error: Do not know how to get inherited 2D scale for this `scale `object type!"));
        }

        // Scale transformation due to parent scale
        engine_math_scale_point(&inheritable->px, &inheritable->py, 0, 0, temp_parent_scale_x, temp_parent_scale_y);

        inheritable->px += temp_parent_pos_x;
        inheritable->py += temp_parent_pos_y;
        inheritable->rotation += temp_parent_rot;
        inheritable->sx *= temp_parent_scale_x;
        inheritable->sy *= temp_parent_scale_y;
        inheritable->opacity *= parent_opacity;

        // Rotate child around parent
        engine_math_rotate_point(&inheritable->px, &inheritable->py, temp_parent_pos_x, temp_parent_pos_y, temp_parent_rot);

        current_node_base = current_node_base->parent_node_base;
    }
}


void node_base_set_attr_handler_default(mp_obj_t node_instance){
    if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);
}


void node_base_use_default_attr_handler(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    default_instance_attr_func(self_in, attribute, destination);
}


void node_base_set_attr_handler(mp_obj_t node_instance, mp_attr_fun_t (*attr_handler_func)(mp_obj_t self_in, qstr attribute, mp_obj_t *destination)){
    default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
    MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, attr_handler_func, 5);
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy_all:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_all_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy_children:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_children_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_add_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_child_count:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_count_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_remove_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_layer:
            destination[0] = mp_obj_new_int(self_node_base->layer);
            return true;
        break;
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
            return true;
        break;
        case MP_QSTR_global_position:
        {
            engine_inheritable_2d_t inherited;
            node_base_inherit_2d(self_node_base, &inherited);

            destination[0] = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(inherited.px), mp_obj_new_float(inherited.py)});

            return true;
        }
        break;
    }

    return false;
}

// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool node_base_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR_layer:
            node_base_set_layer(self_node_base, mp_obj_get_int(destination[1]));
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("NodeBase: ERROR: Setting the global position of a node is not supported yet!"));
        break;
    }

    return false;
}


void node_base_attr_handler(mp_obj_t self, qstr attr, mp_obj_t *dest,
                            attr_handler_func load_functions[],
                            attr_handler_func store_functions[],
                            uint8_t attr_function_count){

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self, &is_obj_instance);

    // Pointer to function that we're going
    // to call in the inner loop
    attr_handler_func current_attr_function = NULL;

    for(uint8_t i=0; i<attr_function_count; i++){
        // Check if a load or store operation, set flag depending
        bool is_store = false;
        if(dest[0] == MP_OBJ_NULL){          // Load
            current_attr_function = load_functions[i];
            is_store = false;
        }else if(dest[1] != MP_OBJ_NULL){    // Store
            current_attr_function = store_functions[i];
            is_store = true;
        }

        // Try to use the attr function to handle the operation
        bool attr_handled = current_attr_function(node_base, attr, dest);

        // If handled, stop
        if(attr_handled){
            // If this was a store operation, mark it as a success
            if(is_store) dest[0] = MP_OBJ_NULL;
            return;
        }
    }

    // If this is a Python class instance and the attr was NOT
    // handled by all the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance){
        node_base_use_default_attr_handler(self, attr, dest);
    }
}