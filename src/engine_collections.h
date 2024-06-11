#ifndef ENGINE_COLLECTIONS_H
#define ENGINE_COLLECTIONS_H

#include "nodes/3D/camera_node.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "py/obj.h"

/*
    Certain aspects of the engine rely on being able to
    loop through certain objects quickly. For example,
    it would be a waste of time to loop through all nodes every
    time a node needs a camera to render itself, it's because of
    this that duplicate references are kept in shorter lists
*/

linked_list_node *engine_collections_track_camera(engine_node_base_t *camera_node_base);
void engine_collections_untrack_camera(linked_list_node *camera_list_node);

linked_list_node *engine_collections_track_physics(engine_node_base_t *physics_node_base);
void engine_collections_untrack_physics(linked_list_node *physics_list_node);

linked_list_node *engine_collections_track_gui(engine_node_base_t *gui_node_base);
void engine_collections_untrack_gui(linked_list_node *gui_list_node);

linked_list_node *engine_collections_track_deletable(engine_node_base_t *node_base);

linked_list *engine_collections_get_camera_list();
linked_list *engine_collections_get_physics_list();
linked_list *engine_collections_get_gui_list();
linked_list *engine_collections_get_deletable_list();


#endif