#include "engine_animation_delay.h"
#include "debug/debug_print.h"
#include "py/objtype.h"
#include "engine_animation_module.h"
#include "nodes/node_base.h"
#include "utility/engine_mp.h"

#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_color.h"
#include <string.h>


/* --- doc ---
   NAME: tick
   ID: delay_tick
   DESC: Main tick callback function of Delay. Override this in a class or set to a function that accepts a parameter dt to redefine how the delay works. After `delay` time the `after` function is called by default.
   PARAM: [type=float]       [name=dt]          [value=time in milliseconds]
   RETURN: None
*/
STATIC mp_obj_t delay_class_tick(mp_obj_t self_in, mp_obj_t dt_obj){
    ENGINE_INFO_PRINTF("Delay: tick!");

    delay_class_obj_t *delay = NULL;

    // Depending on what the user did and whats in ->self, get the base back
    if(mp_obj_is_instance_type(((mp_obj_base_t*)self_in)->type)){
        delay = mp_load_attr(self_in, MP_QSTR_base);
    }else{
        delay = self_in;
    }

    // Don't do anything if already done
    if(delay->finished == true){
        return mp_const_none;
    }

    // Get dt and add to total running time
    float dt = mp_obj_get_float(dt_obj);
    delay->time += dt;

    // If reached end of time, mark as finished
    // and stop
    if(delay->time >= delay->delay){
        delay->finished = true;
        delay->time = 0.0f;

        // Call the maybe defined `after` function
        if(delay->after != mp_const_none){
            mp_obj_t exec[2];
            exec[0] = delay->after;
            exec[1] = delay->self;
            mp_call_method_n_kw(0, 0, exec);
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(delay_class_tick_obj, delay_class_tick);


/* --- doc ---
   NAME: start
   ID: delay_start
   DESC: Starts a delay that will call a function after a certain amount of time
   PARAM: [type=float]       [name=delay]          [value=any value in milliseconds]
   PARAM: [type=function]    [name=after]          [value=function to be called after delay]
   RETURN: None
*/
mp_obj_t delay_class_start(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("Delay: start");

    // self, always `delay_class_obj_t` since attr function handles getting base
    delay_class_obj_t *delay = args[0];

    delay->finished = false;
    delay->time = 0.0f;

    // Handle setting the delay
    if(n_args >= 2){
        delay->delay = mp_obj_get_float(args[1]);
    }

    // Handle setting the `after` function
    if(n_args >= 3){
        delay->after = args[2];
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(delay_class_start_obj, 2, 3, delay_class_start);


mp_obj_t delay_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Delay: Deleted");

    delay_class_obj_t *delay = self_in;
    engine_animation_untrack(delay->list_node);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(delay_class_del_obj, delay_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool delay_load_attr(delay_class_obj_t *delay, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&delay_class_del_obj);
            destination[1] = delay;
            return true;
        break;
        case MP_QSTR_start:
            destination[0] = MP_OBJ_FROM_PTR(&delay_class_start_obj);
            destination[1] = delay;
            return true;
        break;
        case MP_QSTR_tick:
            destination[0] = delay->tick;
            destination[1] = delay->self;      // Pass either native or instance class object depending
            return true;
        break;
        case MP_QSTR_after:
            destination[0] = delay->after;
            destination[1] = delay->self;      // Pass either native or instance class object depending
            return true;
        break;
        case MP_QSTR_base:
            destination[0] = delay;
            return true;
        break;
        case MP_QSTR_delay:
            destination[0] = mp_obj_new_float(delay->delay);
            return true;
        break;
        case MP_QSTR_finished:
            destination[0] = mp_obj_new_bool(delay->finished);
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool delay_store_attr(delay_class_obj_t *delay, qstr attribute, mp_obj_t *destination){
    switch(attribute){
        case MP_QSTR_delay:
            delay->delay = mp_obj_get_float(destination[1]);
            return true;
        break;
        case MP_QSTR_finished:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Delay: Changing the value of `finished` is not allowed!"));
            return true;
        break;
        case MP_QSTR_tick:
            delay->tick = destination[1];
            return true;
        break;
        case MP_QSTR_after:
            delay->after = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


/* --- doc ---
   NAME: after
   ID: delay_after
   DESC: After `delay` amount of time this callback function is called
   RETURN: None
*/
STATIC mp_attr_fun_t delay_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsCircle2DNode attr");

    bool is_instance = mp_obj_is_instance_type(((mp_obj_base_t*)self_in)->type);
    delay_class_obj_t *self;

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
        attr_handled = delay_load_attr(self, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = delay_store_attr(self, attribute, destination);

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



/* --- doc ---
   NAME: Delay
   ID: Delay
   DESC: Object for executing code after a certain non-blocking delay
   ATTR:    [type=function]            [name={ref_link:delay_start}]  [value=function]
   ATTR:    [type=function]            [name={ref_link:delay_tick}]   [value=function]
   ATTR:    [type=function]            [name={ref_link:delay_after}]  [value=function]
   ATTR:    [type=float]               [name=delay]                   [value=any positive value representing milliseconds]
   ATTR:    [type=boolean]             [name=finished]                [value=True or False (read-only)]
   OVRR:    [type=function]            [name={ref_link:delay_tick}]   [value=function]
   OVRR:    [type=function]            [name={ref_link:delay_after}]  [value=function]
*/
mp_obj_t delay_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Delay");

    bool inherited = false;

    if(n_args == 1 && mp_obj_is_instance_type(((mp_obj_base_t*)args[0])->type)){
        inherited = true;
    }else{
        inherited = false;
    }

    delay_class_obj_t *self = m_new_obj_with_finaliser(delay_class_obj_t);
    self->self = self;
    self->tick = MP_OBJ_FROM_PTR(&delay_class_tick_obj);
    self->list_node = engine_animation_track(self);
    self->base.type = &delay_class_type;

    self->delay = 1000.0f;
    self->time = 0.0f;
    self->finished = true;
    self->after = mp_const_none;

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t child_class_obj = args[0];

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)child_class_obj)->type, attr, default_instance_attr_func, 5);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(child_class_obj, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            self->tick = MP_OBJ_FROM_PTR(&delay_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            self->tick = dest[0];
        }

        mp_load_method_maybe(child_class_obj, MP_QSTR_after, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            self->after = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            self->after = dest[0];
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
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)child_class_obj)->type, attr, delay_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        self->self = child_class_obj;
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class attributes
STATIC const mp_rom_map_elem_t delay_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(delay_class_locals_dict, delay_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    delay_class_type,
    MP_QSTR_Delay,
    MP_TYPE_FLAG_NONE,

    make_new, delay_class_new,
    attr, delay_class_attr,
    locals_dict, &delay_class_locals_dict
);