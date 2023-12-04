#include "node_base.h"
#include "engine_object_layers.h"


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

            // This linked list is being deleted, free each nodes as we go
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


void node_base_get_child_absolute_xy(int32_t *x, int32_t *y, mp_obj_t child_node_base_in){
    *x = 0;
    *y = 0;

    engine_node_base_t *child_node_base = child_node_base_in;
    engine_node_base_t *parent_node_base = child_node_base->parent_node_base;

    // Keep going up until we reach the top-most parent (then we'll work out way back down)
    while(parent_node_base != NULL){
        parent_node_base = parent_node_base->parent_node_base;
    }

    
}