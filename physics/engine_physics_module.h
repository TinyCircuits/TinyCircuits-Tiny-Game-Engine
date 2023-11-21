#ifndef ENGINE_PHYSICS_MODULE_H
#define ENGINE_PHYSICS_MODULE_H

#include "utility/linked_list.h"
#include "nodes/node_base.h"

// Physics are nodes just like everything else but are tracked
// new a different list that the engine can loop through quickly.
// This avoids a search through all the nodes when copying attributes
linked_list_node *engine_physics_track_node(engine_node_base_t *obj);

// Call this with the saved link_list_node pointer to
// remove the physics node from the list of tracked nodes
void engine_physics_untrack_node(linked_list_node *physics_list_node);


#endif