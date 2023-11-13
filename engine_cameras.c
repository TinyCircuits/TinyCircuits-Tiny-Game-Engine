#include "engine_cameras.h"
#include "debug/debug_print.h"


// A linked list of cameras to loop through to render
// all object layers for each camera
linked_list engine_cameras;


linked_list_node *engine_camera_track(engine_node_base_t *obj){
    ENGINE_INFO_PRINTF("Tracking new camera");
    return linked_list_add_obj(&engine_cameras, obj);
}


void engine_camera_untrack(linked_list_node *camera_list_node){
    ENGINE_INFO_PRINTF("Untracking camera");
    linked_list_del_list_node(&engine_cameras, camera_list_node);
}


void engine_camera_draw_for_each(mp_obj_t dest[2]){
    linked_list_node *current_camera_list_node = engine_cameras.start;
    if(current_camera_list_node == NULL){
        ENGINE_WARNING_PRINTF("No cameras exist, not calling draw callbacks!");
    }

    mp_obj_t arguments[3];
    arguments[0] = dest[0];
    arguments[1] = dest[1];

    while(current_camera_list_node != NULL){
        engine_node_base_t *node_base = current_camera_list_node->object;

        if(node_base->inherited){
            arguments[2] = node_base->node;
        }else{
            arguments[2] = node_base;
        }
        
        mp_call_method_n_kw(1, 0, arguments);
        current_camera_list_node = current_camera_list_node->next;
    }
}