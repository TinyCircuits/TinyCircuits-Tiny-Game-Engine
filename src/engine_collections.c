#include "engine_collections.h"



linked_list_node *engine_collections_track_camera(engine_node_base_t *camera_node_base){
    return linked_list_add_obj(&engine_camera_nodes_collection, camera_node_base);
}

void engine_collections_untrack_camera(linked_list_node *camera_list_node){
    linked_list_del_list_node(&engine_camera_nodes_collection, camera_list_node);
}


linked_list_node *engine_collections_track_physics(engine_node_base_t *physics_node_base){
    return linked_list_add_obj(&engine_physics_nodes_collection, physics_node_base);
}

void engine_collections_untrack_physics(linked_list_node *physics_list_node){
    linked_list_del_list_node(&engine_physics_nodes_collection, physics_list_node);
}


linked_list_node *engine_collections_track_gui(engine_node_base_t *gui_node_base){
    return linked_list_add_obj(&engine_gui_nodes_collection, gui_node_base);
}

void engine_collections_untrack_gui(linked_list_node *gui_list_node){
    linked_list_del_list_node(&engine_gui_nodes_collection, gui_list_node);
}