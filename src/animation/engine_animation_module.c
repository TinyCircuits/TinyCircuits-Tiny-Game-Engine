#include "engine_animation_module.h"
#include "py/obj.h"
#include "engine_main.h"


// Holds a list of Tween and Delay
linked_list animation_list;


linked_list_node* engine_animation_track(mp_obj_t *animation_element){
    return linked_list_add_obj(&animation_list, animation_element);
}


void engine_animation_untrack(linked_list_node *list_node){
    linked_list_del_list_node(&animation_list, list_node);
}


void engine_animation_init(){
    linked_list_init(&animation_list);
}


void engine_animation_tick(float dt){
    linked_list_node *current = animation_list.start;
    mp_obj_t exec[3];

    while(current != NULL){
        mp_obj_t element = current->object;

        if(mp_obj_is_type(element, &tween_class_type)){
            tween_class_obj_t *tween = current->object;

            exec[0] = tween->tick;
            exec[1] = tween->self;
            exec[2] = mp_obj_new_float(dt);

            mp_call_method_n_kw(1, 0, exec);
        }else{
            delay_class_obj_t *delay = current->object;

            exec[0] = delay->tick;
            exec[1] = delay->self;
            exec[2] = mp_obj_new_float(dt);

            mp_call_method_n_kw(1, 0, exec);
        }

        current = current->next;
    }
}


STATIC mp_obj_t engine_animation_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_animation_module_init_obj, engine_animation_module_init);


/* --- doc ---
   NAME: engine_animation
   DESC: Module for animating certain aspects of the engine
   ATTR: [type=object]     [name={ref_link:Tween}]      [value=object] 
   ATTR: [type=object]     [name={ref_link:Delay}]      [value=object] 
   ATTR: [type=enum/int]   [name=LOOP]                  [value=1]
   ATTR: [type=enum/int]   [name=ONE_SHOT]              [value=2]
   ATTR: [type=enum/int]   [name=PING_PONG]             [value=3]
*/
STATIC const mp_rom_map_elem_t engine_animation_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_animation) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_animation_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Tween), (mp_obj_t)&tween_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_Delay), (mp_obj_t)&delay_class_type},
    { MP_ROM_QSTR(MP_QSTR_LOOP), MP_ROM_INT(engine_animation_loop_loop) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(engine_animation_loop_one_shot) },
    { MP_ROM_QSTR(MP_QSTR_PING_PONG), MP_ROM_INT(engine_animation_loop_ping_pong) },

    { MP_ROM_QSTR(MP_QSTR_EASE_LINEAR), MP_ROM_INT(engine_animation_ease_linear) },

    { MP_ROM_QSTR(MP_QSTR_EASE_SINE_IN), MP_ROM_INT(engine_animation_ease_sine_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_SINE_OUT), MP_ROM_INT(engine_animation_ease_sine_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_SINE_IN_OUT), MP_ROM_INT(engine_animation_ease_sine_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_QUAD_IN), MP_ROM_INT(engine_animation_ease_quad_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUAD_OUT), MP_ROM_INT(engine_animation_ease_quad_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUAD_IN_OUT), MP_ROM_INT(engine_animation_ease_quad_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_CUBIC_IN), MP_ROM_INT(engine_animation_ease_cubic_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_CUBIC_OUT), MP_ROM_INT(engine_animation_ease_cubic_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_CUBIC_IN_OUT), MP_ROM_INT(engine_animation_ease_cubic_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_QUART_IN), MP_ROM_INT(engine_animation_ease_quart_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUART_OUT), MP_ROM_INT(engine_animation_ease_quart_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUART_IN_OUT), MP_ROM_INT(engine_animation_ease_quart_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_QUINT_IN), MP_ROM_INT(engine_animation_ease_quint_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUINT_OUT), MP_ROM_INT(engine_animation_ease_quint_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_QUINT_IN_OUT), MP_ROM_INT(engine_animation_ease_quint_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_EXP_IN), MP_ROM_INT(engine_animation_ease_exp_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_EXP_OUT), MP_ROM_INT(engine_animation_ease_exp_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_EXP_IN_OUT), MP_ROM_INT(engine_animation_ease_exp_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_CIRC_IN), MP_ROM_INT(engine_animation_ease_circ_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_CIRC_OUT), MP_ROM_INT(engine_animation_ease_circ_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_CIRC_IN_OUT), MP_ROM_INT(engine_animation_ease_circ_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_BACK_IN), MP_ROM_INT(engine_animation_ease_back_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_BACK_OUT), MP_ROM_INT(engine_animation_ease_back_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_BACK_IN_OUT), MP_ROM_INT(engine_animation_ease_back_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_ELAST_IN), MP_ROM_INT(engine_animation_ease_elast_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_ELAST_OUT), MP_ROM_INT(engine_animation_ease_elast_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_ELAST_IN_OUT), MP_ROM_INT(engine_animation_ease_elast_in_out) },

    { MP_ROM_QSTR(MP_QSTR_EASE_BOUNCE_IN), MP_ROM_INT(engine_animation_ease_bounce_in) },
    { MP_ROM_QSTR(MP_QSTR_EASE_BOUNCE_OUT), MP_ROM_INT(engine_animation_ease_bounce_out) },
    { MP_ROM_QSTR(MP_QSTR_EASE_BOUNCE_IN_OUT), MP_ROM_INT(engine_animation_ease_bounce_in_out) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_animation_globals, engine_animation_globals_table);

const mp_obj_module_t engine_animation_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_animation_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_animation, engine_animation_user_cmodule);
