#include "engine_animation_module.h"
#include "py/obj.h"

linked_list tween_list;


linked_list_node* engine_animation_track_tween(tween_class_obj_t *tween){
    return linked_list_add_obj(&tween_list, tween);
}


void engine_animation_untrack_tween(linked_list_node *list_node){
    linked_list_del_list_node(&tween_list, list_node);
}


void engine_animation_init(){
    linked_list_init(&tween_list);
}


void engine_animation_tick(float dt){
    linked_list_node *current = tween_list.start;
    mp_obj_t exec[3];

    while(current != NULL){
        tween_class_obj_t *tween = current->object;

        exec[0] = tween->tick;
        exec[1] = tween->self;
        exec[2] = mp_obj_new_float(dt);
        mp_call_method_n_kw(1, 0, exec);

        current = current->next;
    }
}


/* --- doc ---
   NAME: engine_animation
   DESC: Module for animating certain aspects of the engine
   ATTR: [type=object]     [name={ref_link:Tween}]      [value=object] 
   ATTR: [type=enum/int]   [name=LOOP]                  [value=1]
   ATTR: [type=enum/int]   [name=ONE_SHOT]              [value=2]
   ATTR: [type=enum/int]   [name=PING_PONG]             [value=3]
*/
STATIC const mp_rom_map_elem_t engine_animation_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_animation) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Tween), (mp_obj_t)&tween_class_type},
    { MP_ROM_QSTR(MP_QSTR_LOOP), MP_ROM_INT(engine_animation_loop) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(engine_animation_one_shot) },
    { MP_ROM_QSTR(MP_QSTR_PING_PONG), MP_ROM_INT(engine_animation_ping_pong) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_animation_globals, engine_animation_globals_table);

const mp_obj_module_t engine_animation_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_animation_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_animation, engine_animation_user_cmodule);
