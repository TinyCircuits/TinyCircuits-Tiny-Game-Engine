#include "physics_node_base.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"


// https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Body.h#L35-L39
void physics_node_base_apply_impulse_base(engine_physics_node_base_t *physics_node_base, float impulse_x, float impulse_y, float position_x, float position_y){
    vector2_class_obj_t *physics_node_base_velocity = physics_node_base->velocity;

    physics_node_base_velocity->x.value += physics_node_base->inverse_mass * impulse_x;
    physics_node_base_velocity->y.value += physics_node_base->inverse_mass * impulse_y;

    float cross = engine_math_cross_product_v_v(position_x, position_y, impulse_x, impulse_y);
    physics_node_base->angular_velocity -= physics_node_base->inverse_moment_of_inertia * cross;
}


// Adds a impulse (Vector2)(directly sets velocities): https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Body.h#L35-L39
mp_obj_t physics_node_base_apply_impulse(mp_obj_t self_in, mp_obj_t impulse, mp_obj_t contact_position){
    engine_node_base_t *node_base = self_in;
    engine_physics_node_base_t *physics_node_base = node_base->node;

    if(mp_obj_is_type(impulse, &vector2_class_type) && mp_obj_is_type(contact_position, &vector2_class_type)){
        vector2_class_obj_t *applying_impulse = impulse;
        vector2_class_obj_t *applying_position = contact_position;

        physics_node_base_apply_impulse_base(physics_node_base, applying_impulse->x.value, applying_impulse->y.value, applying_position->x.value, applying_position->y.value);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysicsNodeBase: ERROR: Tried to apply an impulse with variable/object that is not Vector2! (either impulse or position)"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(physics_node_base_apply_impulse_obj, physics_node_base_apply_impulse);


void set_collision_mask(uint32_t *collision_mask, mp_int_t layer, bool state){
    if(layer < 0 || layer > 31){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Layer index out of bounds, should be between 0 ~ 31 (inclusive), got %d"), layer);
    }

    BIT_SET(*collision_mask, layer, state);
}


mp_obj_t physics_node_base_enable_layer(mp_obj_t self_in, mp_obj_t layer_in){
    engine_node_base_t *node_base = self_in;
    engine_physics_node_base_t *physics_node_base = node_base->node;
    set_collision_mask(&physics_node_base->collision_mask, mp_obj_get_int(layer_in), 1);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(physics_node_base_enable_layer_obj, physics_node_base_enable_layer);


mp_obj_t physics_node_base_disable_layer(mp_obj_t self_in, mp_obj_t layer_in){
    engine_node_base_t *node_base = self_in;
    engine_physics_node_base_t *physics_node_base = node_base->node;
    set_collision_mask(&physics_node_base->collision_mask, mp_obj_get_int(layer_in), 0);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(physics_node_base_disable_layer_obj, physics_node_base_disable_layer);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    engine_physics_node_base_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_apply_impulse:
            destination[0] = MP_OBJ_FROM_PTR(&physics_node_base_apply_impulse_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_enable_collision_layer:
            destination[0] = MP_OBJ_FROM_PTR(&physics_node_base_enable_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_disable_collision_layer:
            destination[0] = MP_OBJ_FROM_PTR(&physics_node_base_disable_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_on_collide:
            destination[0] = MP_OBJ_FROM_PTR(&self->on_collide_cb);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_on_separate:
            destination[0] = MP_OBJ_FROM_PTR(&self->on_separate_cb);
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
            destination[0] = mp_obj_new_float(self->angular_velocity);
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = mp_obj_new_float(self->rotation);
            return true;
        break;
        case MP_QSTR_friction:
            destination[0] = self->friction;
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
        case MP_QSTR_outline:
            destination[0] = self->outline;
            return true;
        break;
        case MP_QSTR_outline_color:
            destination[0] = self->outline_color;
            return true;
        break;
        case MP_QSTR_collision_mask:
            destination[0] = mp_obj_new_int(self->collision_mask);
            return true;
        break;
        default:
            return false; // Fail
    }

    return false;
}


/*  --- doc ---
    NAME: on_collide
    ID: on_collide
    DESC: Callback that is invoked when physics nodes collide
    PARAM: [type=object]                            [name=self]         [value=object]
    PARAM: [type={ref_link:CollisionContact2D}]     [name=contact]      [value={ref_link:CollisionContact2D}]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_separate
    ID: on_separate
    DESC: Callback that is invoked when a physics node stops colliding
    PARAM: [type=object]                            [name=self]         [value=object]
    RETURN: None
*/
/*  --- doc ---
    NAME: physics_tick
    ID: physics_tick
    DESC: Overridable physics tick callback that happens before collision and node tick() callbacks
    PARAM: [type=object] [name=self] [value=object]
    PARAM: [type=float]  [name=dt]   [value=positive float in seconds]
    RETURN: None
*/
/*  --- doc ---
    NAME: enable_collision_layer
    ID: enable_collision_layer
    DESC: Allow this physics node to collide with other nodes on this layer
    PARAM: [type=int]  [name=layer]   [value=0 ~ 31]
    RETURN: None
*/
/*  --- doc ---
    NAME: disable_collision_layer
    ID: disable_collision_layer
    DESC: Disallow this physics node from colliding with other nodes on this layer
    PARAM: [type=int]  [name=layer]   [value=0 ~ 31]
    RETURN: None
*/


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool physics_node_base_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_collide:
            self->on_collide_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_separate:
            self->on_separate_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_velocity:
            self->velocity = destination[1];
            return true;
        break;
        case MP_QSTR_angular_velocity:
            self->angular_velocity = mp_obj_get_float(destination[1]);
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = mp_obj_get_float(destination[1]);
            return true;
        break;
        case MP_QSTR_friction:
            self->friction = destination[1];
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
        case MP_QSTR_outline:
            self->outline = destination[1];
            return true;
        break;
        case MP_QSTR_outline_color:
            self->outline_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_collision_mask:
            self->collision_mask = mp_obj_get_int(destination[1]);
            return true;
        break;
        default:
            return false; // Fail
    }

    return false;
}