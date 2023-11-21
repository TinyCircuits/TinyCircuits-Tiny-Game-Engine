#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include "nodes/2d/physics_2d_node.h"

void engine_physics_init();

void engine_physics_tick();

void engine_physics_create_rectangle_body(engine_physics_2d_node_common_data_t *common_data);

#endif  // ENGINE_PHYSICS_H