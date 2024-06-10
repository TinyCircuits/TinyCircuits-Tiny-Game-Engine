#include "engine_physics_ids.h"
#include "py/obj.h"
#include "py/runtime.h"

// Buffer of available physics IDs. Filled with values
// 1 -> `PHYSICS_ID_MAX` on physics `init`. 
uint8_t available_physics_ids[PHYSICS_ID_MAX];
uint8_t next_available_physics_id_index = 0;


void engine_physics_ids_init(){
    for(uint8_t idx=0; idx<PHYSICS_ID_MAX; idx++){
        available_physics_ids[idx] = idx+1;
    }
}


uint8_t engine_physics_ids_take_available(){
    if(next_available_physics_id_index >= PHYSICS_ID_MAX){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Ran out of IDs to give physics nodes..."));
    }

    uint8_t id = available_physics_ids[next_available_physics_id_index];
    next_available_physics_id_index++;
    return id;
}


void engine_physics_ids_give_back(uint8_t id){
    next_available_physics_id_index--;
    available_physics_ids[next_available_physics_id_index] = id;
}


uint32_t engine_physics_ids_get_pair_index(uint8_t a_id, uint8_t b_id){
    return PHYSICS_ID_MAX * (uint32_t)a_id + (uint32_t)b_id;
}