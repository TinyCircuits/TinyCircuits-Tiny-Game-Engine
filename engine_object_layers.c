#include "engine_object_layers.h"
#include "nodes/base_node.h"

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
    linked_list_node *current_node = NULL;

    for(uint16_t ilx=0; ilx<engine_object_layer_count; ilx++){
        ENGINE_INFO_PRINTF("Starting on objects in layer %d/%d", ilx, engine_object_layer_count-1);

        current_node = engine_object_layers[ilx].start;

        while(current_node != NULL){
            engine_base_node_class_obj_t *current_node_base_class_instance = ((engine_base_node_class_obj_t*)current_node->object);

            // Don't call node callbacks unless it was not just added (callbacks can add nodes, don't want to call the next callbacks until next cycle)
            if(current_node_base_class_instance->just_added == false){
                mp_call_method_n_kw(0, 0, current_node_base_class_instance->tick_dest);
            }else{
                ENGINE_INFO_PRINTF("Did not call node callbacks, it was just added, will call them next game cycle");
                current_node_base_class_instance->just_added = false;
            }

            current_node = current_node->next;
        }
    }

    ENGINE_INFO_PRINTF("##### GAME CYCLE COMPLETE #####\n");
}