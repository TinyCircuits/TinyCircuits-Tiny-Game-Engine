#include "engine_animation_tween.h"
#include "debug/debug_print.h"
#include "py/objtype.h"
#include "engine_animation_module.h"
#include "nodes/node_base.h"
#include "utility/engine_mp.h"

#include "math/vector2.h"
#include "math/vector3.h"

enum tween_value_types {tween_type_float, tween_type_vec2, tween_type_vec3, tween_type_color_rgb};

// Class required functions
STATIC void tween_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    tween_class_obj_t *self = self_in;
    ENGINE_FORCE_PRINTF("print(): Tween");
}


STATIC mp_obj_t tween_class_tick(mp_obj_t self_in, mp_obj_t dt_obj){
    ENGINE_INFO_PRINTF("Tween: tick!");
    tween_class_obj_t *tween = self_in;

    if(tween->finished){
        if(tween->loop_type == engine_animation_one_shot){
            return mp_const_none;
        }else if(tween->value != mp_const_none){
            tween->finished = false;
            tween->time = 0.0f;
        }else{
            return mp_const_none;
        }
    }

    // Get dt and add to total runnign time
    float dt = mp_obj_get_float(dt_obj);
    tween->time += dt;

    // If reached end of time, mark as finished
    // and stop. This lets the user catch the
    // `finished` flag for `loop` and `one_shot`
    if(tween->time >= tween->duration){
        tween->finished = true;
        tween->time = 0.0f;
        return mp_const_none;
    }

    // Figure out where we are in interpolation (percentage)
    float t = tween->time / tween->duration;

    if(tween->tween_type == tween_type_float){
        ((mp_obj_float_t*)(tween->value))->value = tween->initial_0 + ((tween->end_0 - tween->initial_0) * t);
    }else if(tween->tween_type == tween_type_vec2){
        vector2_class_obj_t *value = tween->value;

        float x0 = tween->initial_0;
        float y0 = tween->initial_1;

        float x1 = tween->end_0;
        float y1 = tween->end_1;

        // https://stackoverflow.com/a/51067982
        value->x.value = x0 + t * (x1 - x0);
        value->y.value = y0 + t * (y1 - y0);
    }else if(tween->tween_type == tween_type_vec3){
        vector3_class_obj_t *value = tween->value;

        float x0 = tween->initial_0;
        float y0 = tween->initial_1;
        float z0 = tween->initial_2;

        float x1 = tween->end_0;
        float y1 = tween->end_1;
        float z1 = tween->end_2;

        // https://stackoverflow.com/a/51067982
        value->x.value = x0 + t * (x1 - x0);
        value->y.value = y0 + t * (y1 - y0);
        value->z.value = z0 + t * (z1 - z0);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(tween_class_tick_obj, tween_class_tick);


mp_obj_t tween_class_start(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("Tween: play");

    if(n_args < 4){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tween: ERROR: Not enough arguments passed to `play()`, expected at least 4"));
    }

    tween_class_obj_t *tween = args[0];
    tween->value = args[1];

    mp_obj_type_t *value_type = mp_obj_get_type(args[1]);
    mp_obj_type_t *start_type = mp_obj_get_type(args[2]);
    mp_obj_type_t *end_type = mp_obj_get_type(args[3]);

    if(value_type == &mp_type_float && start_type == &mp_type_float && end_type == &mp_type_float){
        tween->initial_0  = mp_obj_get_float(args[2]);
        tween->end_0      = mp_obj_get_float(args[3]);
        tween->tween_type = tween_type_float;
    }else if(value_type == &vector2_class_type && start_type == &vector2_class_type && end_type == &vector2_class_type){
        vector2_class_obj_t *start = args[2];
        vector2_class_obj_t *end = args[3];

        tween->initial_0 = start->x.value;
        tween->initial_1 = start->y.value;

        tween->end_0 = end->x.value;
        tween->end_1 = end->y.value;

        tween->tween_type = tween_type_vec2;
    }else if(value_type == &vector3_class_type && start_type == &vector3_class_type && end_type == &vector3_class_type){
        vector3_class_obj_t *start = args[2];
        vector3_class_obj_t *end = args[3];

        tween->initial_0 = start->x.value;
        tween->initial_1 = start->y.value;
        tween->initial_2 = start->z.value;

        tween->end_0 = end->x.value;
        tween->end_1 = end->y.value;
        tween->end_2 = end->z.value;

        tween->tween_type = tween_type_vec3;
    }else{
        ENGINE_PRINTF("ERROR: Got types value: %s, start: %s, end %s:", mp_obj_get_type_str(args[1]), mp_obj_get_type_str(args[2]), mp_obj_get_type_str(args[3]));
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tween: ERROR: Unknown combination of `value`, `start`, and `end` object types"));
    }

    if(n_args >= 5){
        tween->duration = mp_obj_get_float(args[4]);
    }

    if(n_args >= 6){
        tween->loop_type = mp_obj_get_int(args[5]);

        if(tween->loop_type > 1){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tween: ERROR: Unknown loop type..."));
        }
    }

    tween->finished = false;
    tween->time = 0.0f;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(tween_class_start_obj, 4, 6, tween_class_start);


mp_obj_t tween_class_pause(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Tween: pause");
    tween_class_obj_t *tween = self_in;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tween_class_pause_obj, tween_class_pause);


mp_obj_t tween_class_restart(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Tween: restart");
    tween_class_obj_t *tween = self_in;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tween_class_restart_obj, tween_class_restart);


mp_obj_t tween_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Tween: Deleted");

    tween_class_obj_t *tween = self_in;
    engine_animation_untrack_tween(tween->list_node);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tween_class_del_obj, tween_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool tween_load_attr(tween_class_obj_t *tween, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&tween_class_del_obj);
            destination[1] = tween;
            return true;
        break;
        case MP_QSTR_start:
            destination[0] = MP_OBJ_FROM_PTR(&tween_class_start_obj);
            destination[1] = tween;
            return true;
        break;
        case MP_QSTR_pause:
            destination[0] = MP_OBJ_FROM_PTR(&tween_class_pause_obj);
            destination[1] = tween;
            return true;
        break;
        case MP_QSTR_restart:
            destination[0] = MP_OBJ_FROM_PTR(&tween_class_restart_obj);
            destination[1] = tween;
            return true;
        break;
        case MP_QSTR_base:
            destination[0] = tween;
            return true;
        break;
        case MP_QSTR_duration:
            destination[0] = mp_obj_new_float(tween->duration);
            return true;
        break;
        case MP_QSTR_loop_type:
            destination[0] = mp_obj_new_int(tween->loop_type);
            return true;
        break;
        case MP_QSTR_finished:
            destination[0] = mp_obj_new_bool(tween->finished);
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool tween_store_attr(tween_class_obj_t *tween, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR_duration:
            tween->duration = mp_obj_get_float(destination[1]);
            return true;
        break;
        case MP_QSTR_loop_type:
            tween->loop_type = mp_obj_get_int(destination[1]);
            return true;
        break;
        case MP_QSTR_finished:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tween: Changing the value of `finished` is not allowed!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t tween_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsCircle2DNode attr");

    bool is_instance = mp_obj_is_instance_type(((mp_obj_base_t*)self_in)->type);
    tween_class_obj_t *self;

    if(is_instance){
        mp_obj_t dest[2];
        dest[0] = MP_OBJ_NULL; // Indicate we want to load a value
        default_instance_attr_func(self_in, MP_QSTR_base, dest);
        self = dest[0];
    }else{
        self = self_in;
    }

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = tween_load_attr(self, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = tween_store_attr(self, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }

    return mp_const_none;
}


mp_obj_t tween_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Tween");

    bool inherited = false;

    if(n_args == 1 && mp_obj_is_instance_type(((mp_obj_base_t*)args[0])->type)){
        inherited = true;
    }else{
        inherited = false;
    }

    tween_class_obj_t *self = m_new_obj_with_finaliser(tween_class_obj_t);
    self->self = self;
    self->tick = MP_OBJ_FROM_PTR(&tween_class_tick_obj);
    self->list_node = engine_animation_track_tween(self);
    self->base.type = &tween_class_type;

    self->loop_type = engine_animation_loop;
    self->duration = 1000.0f;
    self->time = 0.0f;
    self->finished = true;
    self->value = mp_const_none;


    if(inherited == true){
        // Get the Python class instance
        mp_obj_t child_class_obj = args[0];

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(child_class_obj, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            self->tick = MP_OBJ_FROM_PTR(&tween_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            self->tick = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(child_class_obj, MP_QSTR_base, self);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)child_class_obj)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)child_class_obj)->type, attr, tween_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        self->self = child_class_obj;
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class attributes
STATIC const mp_rom_map_elem_t tween_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(tween_class_locals_dict, tween_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    tween_class_type,
    MP_QSTR_Tween,
    MP_TYPE_FLAG_NONE,

    make_new, tween_class_new,
    print, tween_class_print,
    attr, tween_class_attr,
    locals_dict, &tween_class_locals_dict
);