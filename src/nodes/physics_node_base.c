#include "physics_node_base.h"
#include "math/vector2.h"
#include "math/engine_math.h"


float physics_node_base_calculate_inverse_mass(engine_physics_node_base_t *physics_node_base){
    float mass = mp_obj_get_float(physics_node_base->mass);
    
    if(mass == 0.0f){
        physics_node_base->inverse_mass = 0.0f;
    }else{
        physics_node_base->inverse_mass = 1.0f / mass;
    }
}


void physics_node_base_apply_impulse_base(engine_physics_node_base_t *physics_node_base, float impulse_x, float impulse_y, float position_x, float position_y){
    vector2_class_obj_t *physics_node_base_velocity = physics_node_base->velocity;

    physics_node_base_velocity->x += physics_node_base->inverse_mass * impulse_x;
    physics_node_base_velocity->y += physics_node_base->inverse_mass * impulse_y;

    float cross = engine_math_cross_product(impulse_x, impulse_y, position_x, position_y);
    physics_node_base->angular_velocity = mp_obj_new_float(physics_node_base->inverse_moment_of_inertia * cross);
}


// Adds a impulse (Vector2)(directly sets velocities): https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Body.h#L35-L39
mp_obj_t physics_node_base_apply_impulse(mp_obj_t self_in, mp_obj_t impulse, mp_obj_t contact_position){
    engine_node_base_t *node_base = self_in;
    engine_physics_node_base_t *physics_node_base = node_base->node;

    if(mp_obj_is_type(impulse, &vector2_class_type) && mp_obj_is_type(contact_position, &vector2_class_type)){
        vector2_class_obj_t *applying_impulse = impulse;
        vector2_class_obj_t *applying_position = contact_position;

        physics_node_base_apply_impulse_base(physics_node_base, applying_impulse->x, applying_impulse->y, applying_position->x, applying_position->y);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysicsNodeBase: ERROR: Tried to apply an impulse with variable/object that is not Vector2! (either impulse or position)"));
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(physics_node_base_apply_impulse_obj, physics_node_base_apply_impulse);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_apply_impulse:
            destination[0] = MP_OBJ_FROM_PTR(&physics_node_base_apply_impulse_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_velocity:
            destination[0] = self->velocity;
            return true;
        break;
        case MP_QSTR_angular_velocity:
            destination[0] = self->angular_velocity;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = mp_obj_new_float(self->rotation);
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
        case MP_QSTR_angular_velocity:
            self->angular_velocity = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = mp_obj_get_float(destination[1]);
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