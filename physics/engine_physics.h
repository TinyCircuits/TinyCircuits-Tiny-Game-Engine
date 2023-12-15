#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include "nodes/2d/physics_2d_node.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"

void engine_physics_init();

void engine_physics_tick();


// Physics are nodes just like everything else but are tracked
// new a different list that the engine can loop through quickly.
// This avoids a search through all the nodes when copying attributes
linked_list_node *engine_physics_track_node(engine_node_base_t *obj);

// Call this with the saved link_list_node pointer to
// remove the physics node from the list of tracked nodes
void engine_physics_untrack_node(linked_list_node *physics_list_node);


void engine_physics_sync_engine_nodes_to_bodies();

void engine_physics_sync_bodies_to_engine_nodes();

// Untrack all physics nodes
void engine_physics_clear_all();

#endif  // ENGINE_PHYSICS_H