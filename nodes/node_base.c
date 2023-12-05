#include "node_base.h"
#include "engine_object_layers.h"
#include "math/engine_math.h"
#include "math/vector2.h"


// As we traverse from the top-most parent to the child, each chain
// of children of each parent have to be rotated about the current
// parent as we traverse back down. This all needs to be stored
// somewhere without actually affecting the positions of the nodes
float child_parent_chain_positions_x[NODE_BASE_MAX_CHILD_COUNT];
float child_parent_chain_positions_y[NODE_BASE_MAX_CHILD_COUNT];
float child_parent_chain_positions_rotations_radians[NODE_BASE_MAX_CHILD_COUNT];

mp_obj_t node_base_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Node Base: Deleted (garbage collected, removing self from active engine objects)");

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
    free(node_base->node_common_data);

    return mp_const_none;
}


mp_obj_t node_base_add_child(mp_obj_t self_parent_in, mp_obj_t child_in){
    ENGINE_INFO_PRINTF("Node Base: Adding child...");

    engine_node_base_t *parent_node_base = self_parent_in;
    engine_node_base_t *child_node_base = child_in;

    // Loop up until reach parent then see how many children it has
    uint8_t child_count = 0;
    engine_node_base_t *counting_node_base = parent_node_base->parent_node_base;

    // Keep going up until we reach the top-most parent
    while(counting_node_base != NULL){
        child_count++;
        counting_node_base = counting_node_base->parent_node_base;
    }

    // Raise error if too many children (capping this to reduce number of rotations that need to occur
    // and so that a fixed array can be kept for holding )
    if(child_count >= NODE_BASE_MAX_CHILD_COUNT){
        mp_raise_msg(&mp_type_RuntimeError, "ERROR: Tried to add too many children to this chain");
        return mp_const_none;
    }

    child_node_base->location_in_parents_children = linked_list_add_obj(&parent_node_base->children_node_bases, child_node_base);
    child_node_base->parent_node_base = parent_node_base;

    return mp_const_none;
}


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
        mp_raise_msg(&mp_type_RuntimeError, "Child node does not exist on this parent, cannot remove!");
    }

    return mp_const_none;
}


mp_obj_t node_base_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Node Base: Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}


mp_obj_t node_base_get_layer(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Node Base: Getting object layer...");

    engine_node_base_t *node_base = self_in;
    return mp_obj_new_int(node_base->layer);
}


void node_base_get_child_absolute_xy(float *x, float *y, float *rotation, mp_obj_t child_node_base_in){
    engine_node_base_t *child_node_base = child_node_base_in;

    // Before doing anything, check if this child even has a parent to rotate about
    if(child_node_base->parent_node_base != NULL){
        engine_node_base_t *seeking_node_base = child_node_base;
        int8_t chain_index = -1;

        // Starting at this 'child_node_base_in' (index 0) store/collect all
        // x and y position components + rotation (NOTE: each type of node needs
        // to be handled here! Some can be 3D or just not even have a position)
        while(seeking_node_base != NULL){
            chain_index++;

            vector2_class_obj_t *seeking_node_base_position = mp_load_attr(seeking_node_base->attr_accessor, MP_QSTR_position);
            float seeking_node_base_rotation_radian = mp_obj_get_float(mp_load_attr(seeking_node_base->attr_accessor, MP_QSTR_rotation)) * DEG2RAD;

            child_parent_chain_positions_x[chain_index] = seeking_node_base_position->x;
            child_parent_chain_positions_y[chain_index] = seeking_node_base_position->y;
            child_parent_chain_positions_rotations_radians[chain_index] = seeking_node_base_rotation_radian;

            seeking_node_base = seeking_node_base->parent_node_base;
        }

        float angle_accumulation = 0.0f;

        // Start at the top-most parent in the chain and work our way down
        // the children. For each child rotate all of its children about
        // the current node
        for(uint8_t reverse_chain_index=chain_index; reverse_chain_index>0; reverse_chain_index--){

            // Traverse down starting at the child below 'current_center_node' and rotate each about 'current_center_node'
            for(int8_t child_reverse_chain_index=reverse_chain_index-1; child_reverse_chain_index>=0; child_reverse_chain_index--){
                engine_math_rotate_point(&child_parent_chain_positions_x[child_reverse_chain_index],
                                        &child_parent_chain_positions_y[child_reverse_chain_index],
                                        child_parent_chain_positions_x[reverse_chain_index],
                                        child_parent_chain_positions_y[reverse_chain_index],
                                        child_parent_chain_positions_rotations_radians[reverse_chain_index]);
            }

            angle_accumulation += child_parent_chain_positions_rotations_radians[reverse_chain_index];
        }

        *x = child_parent_chain_positions_x[0];
        *y = child_parent_chain_positions_y[0];
        *rotation = (angle_accumulation+child_parent_chain_positions_rotations_radians[0]) * RAD2DEG;
    }else{
        vector2_class_obj_t *child_node_base_position = mp_load_attr(child_node_base->attr_accessor, MP_QSTR_position);
        float child_node_base_rotation = mp_obj_get_float(mp_load_attr(child_node_base->attr_accessor, MP_QSTR_rotation));
        *x = child_node_base_position->x;
        *y = child_node_base_position->y;
        *rotation = child_node_base_rotation;
    }
}