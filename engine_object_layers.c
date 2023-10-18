#include "engine_object_layers.h"
#include "nodes/minimal_node.h"
#include "nodes/empty_node.h"
#include "nodes/node_types.h"

uint16_t engine_object_layer_count = 8;
linked_list engine_object_layers[8];


linked_list_node *engine_add_object_to_layer(void *obj, uint16_t layer_index){
    if(layer_index >= engine_object_layer_count){
        ENGINE_ERROR_PRINTF("Tried to add object to layer %d but the max layer index is %d. Resize the number of available draw layers at the cost of memory", layer_index, engine_object_layer_count-1);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tried to add object to layer index that is out of bounds! Resize the object layer count!"));
    }

    return linked_list_add_obj(&engine_object_layers[layer_index], obj);
}


void engine_remove_object_from_layer(linked_list_node *object_list_node, uint16_t layer_index){
    linked_list_del_list_node(&engine_object_layers[layer_index], object_list_node);
}


void engine_invoke_all_node_callbacks(){
    linked_list_node *current_linked_list_node = NULL;

    for(uint16_t ilx=0; ilx<engine_object_layer_count; ilx++){
        ENGINE_INFO_PRINTF("Starting on objects in layer %d/%d", ilx, engine_object_layer_count-1);

        current_linked_list_node = engine_object_layers[ilx].start;

        while(current_linked_list_node != NULL){
            // Get minimal version of the current engine node (all nodes should be able to be cast to this)
            engine_minimal_node_class_obj_t *node = ((engine_minimal_node_class_obj_t*)current_linked_list_node->object);

            // As long as this node was not just added, figure out its type and what callbacks it has
            if(node_base_is_just_added(&node->node_base) == false){
                if(node->node_base.type == NODE_TYPE_EMPTY){
                    engine_empty_node_class_obj_t *empty_node = (engine_empty_node_class_obj_t*)node;
                    mp_call_method_n_kw(0, 0, empty_node->tick_dest);
                }
            }else{
                ENGINE_INFO_PRINTF("Did not call node callbacks, it was just added, will call them next game cycle");
                node_base_set_if_just_added(&node->node_base, false);
            }

            current_linked_list_node = current_linked_list_node->next;
        }
    }

    ENGINE_INFO_PRINTF("##### GAME CYCLE COMPLETE #####\n");
}