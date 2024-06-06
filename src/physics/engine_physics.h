#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "math/vector2.h"


float engine_physics_fps_limit_period_ms;
float engine_physics_fps_time_at_last_tick_ms;
float engine_physics_gravity_x;
float engine_physics_gravity_y;


// This should be called when a new physics node is
// created and the result assigned to the node
uint8_t engine_physics_take_available_id();

// This should be called when a physics node is being deleted/collected
// and the assigned ID given back to the ID pool for the future
void engine_physics_give_back_id(uint8_t id);

// Creates array of IDs for physics nodes and
// creates a bit collection for tracking which
// nodes already collided each frame
void engine_physics_init();

void engine_physics_physics_tick(float dt_s);
void engine_physics_tick();

#endif  // ENGINE_PHYSICS_H
