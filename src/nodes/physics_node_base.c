#include "physics_node_base.h"


float physics_node_base_calculate_inverse_mass(engine_physics_node_base_t *physics_node_base){
    float mass = mp_obj_get_float(physics_node_base->mass);
    
    if(mass == 0.0f){
        physics_node_base->inverse_mass = 0.0f;
    }else{
        physics_node_base->inverse_mass = 1.0f / mass;
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_velocity:
            destination[0] = self->velocity;
            return true;
        break;
        case MP_QSTR_acceleration:
            destination[0] = self->acceleration;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_mass:
            destination[0] = self->mass;
            return true;
        break;
        case MP_QSTR_bounciness:
            destination[0] = self->bounciness;
            return true;
        break;
        case MP_QSTR_dynamic:
            destination[0] = self->dynamic;
            return true;
        break;
        case MP_QSTR_solid:
            destination[0] = self->solid;
            return true;
        break;
        case MP_QSTR_gravity_scale:
            destination[0] = self->gravity_scale;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool physics_node_base_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_velocity:
            self->velocity = destination[1];
            return true;
        break;
        case MP_QSTR_acceleration:
            self->acceleration = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_mass:
            self->mass = destination[1];
            physics_node_base_calculate_inverse_mass(self);
            return true;
        break;
        case MP_QSTR_bounciness:
            self->bounciness = destination[1];
            return true;
        break;
        case MP_QSTR_dynamic:
            self->dynamic = destination[1];
            return true;
        break;
        case MP_QSTR_solid:
            self->solid = destination[1];
            return true;
        break;
        case MP_QSTR_gravity_scale:
            self->gravity_scale = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}