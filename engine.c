#include <stdio.h>

#include "engine_object_layers.h"
#include "display/engine_display.h"
#include "input/engine_input_module.h"

// ### MODULE ###

// Module functions


STATIC mp_obj_t engine_init(){
    ENGINE_INFO_PRINTF("Engine init");
    engine_display_init();
    engine_display_send();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_init_obj, engine_init);


STATIC mp_obj_t engine_tick(){
    ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_1, "Loop time");
    ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_1);

    // Update/grab which buttons are pressed before calling all node callbacks
    engine_input_update_pressed_buttons();

    engine_display_dma_wait();

    // Call every instanced node's callbacks
    engine_invoke_all_node_callbacks();

    // After every game cycle send the current active screen buffer to the display
    engine_display_send();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_tick_obj, engine_tick);


STATIC mp_obj_t engine_loop(){
    engine_init();
    ENGINE_INFO_PRINTF("Engine loop starting...");

    while(true){
        engine_tick();
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_loop_obj, engine_loop);


STATIC mp_obj_t engine_total_object_count(){
    return mp_obj_new_int(engine_get_total_object_count());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_total_object_count_obj, engine_total_object_count);



// Module attributes
STATIC const mp_rom_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&engine_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick), (mp_obj_t)&engine_tick_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_loop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_total_object_count), (mp_obj_t)&engine_total_object_count_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);
