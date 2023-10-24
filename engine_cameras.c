#include "engine_cameras.h"

linked_list_node *engine_camera_track(engine_camera_node_class_obj_t *obj){
    ENGINE_INFO_PRINTF("Tracking new camera");
    return linked_list_add_obj(&engine_cameras, obj);
}


void engine_camera_untrack(linked_list_node *camera_list_node){
    ENGINE_INFO_PRINTF("Untracking camera");
    linked_list_del_list_node(&engine_cameras, camera_list_node);
}