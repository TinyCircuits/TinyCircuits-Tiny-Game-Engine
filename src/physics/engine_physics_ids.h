#ifndef ENGINE_PHYSICS_IDS
#define ENGINE_PHYSICS_IDS

#include "utility/engine_bit_collection.h"

// The maximum number of physics nodes that can exist in a scene is restricted
// by the number of IDs available to give those nodes. The number of IDs is
// limited so that IDs can be used with a simple paring function to index in
// a large bit array to check if collisions between objects have already occurred
#define PHYSICS_ID_MAX 180

// Fill `available_physics_ids` with IDs from `1` -> `PHYSICS_ID_MAX`
// through indices `0` -> `PHYSICS_ID_MAX-1`
void engine_physics_ids_init();

// Get the next available ID (probably needed for new objects)
uint8_t engine_physics_ids_take_available();

// Give back an ID to the ID pool (probably had a node get deleted)
void engine_physics_ids_give_back(uint8_t id);

// Get a unique ID/index for a pair of IDS (used for checking if already collided in a bit collection)
// Uses a simple pairing function (https://math.stackexchange.com/a/531914)
uint32_t engine_physics_ids_get_pair_index(uint8_t a_id, uint8_t b_id);

#endif  // ENGINE_PHYSICS_IDS