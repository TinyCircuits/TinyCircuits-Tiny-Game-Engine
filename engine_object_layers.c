#include "engine_object_layers.h"
#include "engine_cameras.h"
#include "nodes/minimal_node.h"
#include "nodes/empty_node.h"
#include "nodes/camera_node.h"
#include "nodes/2d/rectangle_2d_node.h"
#include "nodes/bitmap_sprite_node.h"
#include "nodes/node_types.h"

uint16_t engine_object_layer_count = 8;
linked_list engine_object_layers[8];


uint16_t engine_get_total_object_count(){
    uint16_t count = 0;
    for(uint8_t ilx=0; ilx<engine_object_layer_count; ilx++){
        count += engine_object_layers[ilx].count;
    }

    return count;
}


// Add an object to the pool of all nodes in 'engine_object_layers' at some layer
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


// Go through all nodes and call their callbacks depending on the
// node type. For example, some nodes will only have a 'tick()'
// callback while others will also have a 'draw()' callback.
//
// Nodes with a draw callback will be rendered
void engine_invoke_all_node_callbacks(){
    linked_list_node *current_linked_list_node = NULL;

    for(uint16_t ilx=0; ilx<engine_object_layer_count; ilx++){
        ENGINE_INFO_PRINTF("Starting on objects in layer %d/%d", ilx, engine_object_layer_count-1);

        current_linked_list_node = engine_object_layers[ilx].start;

        while(current_linked_list_node != NULL){
            // Get the base node that every node is stored under
            engine_node_base_t *node_base = current_linked_list_node->object;

            // As long as this node was not just added, figure out its type and what callbacks it has
            if(node_base_is_just_added(node_base) == false){
                switch(node_base->type){
                    // case NODE_TYPE_EMPTY:
                    // {
                    //     // engine_empty_node_class_obj_t *empty_node = (engine_empty_node_class_obj_t*)node_base->node;
                    //     mp_call_method_n_kw(0, 0, node_base->tick_cb);
                    // }
                    // break;
                    // case NODE_TYPE_CAMERA:
                    // {
                    //     // engine_camera_node_class_obj_t *camera_node = (engine_camera_node_class_obj_t*)node_base->node;
                    //     mp_call_method_n_kw(0, 0, node_base->tick_cb);
                    // }
                    break;
                    case NODE_TYPE_RECTANGLE_2D:
                    {
                        // engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = (engine_rectangle_2d_node_class_obj_t*)node_base->node;
                        mp_obj_t exec[2];
                        exec[0] = node_base->tick_cb;
                        exec[1] = node_base->node;
                        mp_call_method_n_kw(0, 0, exec);
                        // engine_camera_draw_for_each(node_base->draw_cb);
                    }
                    break;
                    // case NODE_TYPE_BITMAP_SPRITE:
                    // {
                    //     // engine_bitmap_sprite_node_class_obj_t *bitmap_sprite_node = (engine_bitmap_sprite_node_class_obj_t*)node_base->node;
                    //     mp_call_method_n_kw(0, 0, node_base->tick_cb);
                    //     engine_camera_draw_for_each(node_base->draw_cb);
                    // }
                    // break;
                    default:
                        ENGINE_WARNING_PRINTF("This node type doesn't do anything? %d", node_base->type);
                    break;
                }

            }else{
                ENGINE_INFO_PRINTF("Did not call node callbacks, it was just added, will call them next game cycle");
                node_base_set_if_just_added(node_base, false);
            }

            current_linked_list_node = current_linked_list_node->next;
        }
    }

    ENGINE_INFO_PRINTF("##### GAME CYCLE COMPLETE #####\n");
}