#include <stdio.h>
#include "py/obj.h"
#include "py/gc.h"
#include "engine_main.h"

#include "engine_object_layers.h"
#include "resources/engine_resource_manager.h"
#include "audio/engine_audio_module.h"
#include "input/engine_input_module.h"
#include "display/engine_display.h"
#include "display/engine_display_common.h"
#include "physics/engine_physics.h"
#include "animation/engine_animation_module.h"
#include "engine_gui.h"

#if defined(__arm__)
    #include "hardware/adc.h"
#endif

#define BATTERY_ADC_GPIO_PIN 29
#define BATTERY_ADC_PORT 3

bool is_engine_initialized = false;


void engine_main_raise_if_not_initialized(){
    if(is_engine_initialized == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineMain: ERROR: `import engine_main` needs to be done at least once at the very start!"));
    }
}


void engine_main_reset(){
    ENGINE_PRINTF("EngineMain: Resetting engine...\n");

    // Set back to default
    engine_display_set_fill_color(0x0000);

    // Reset contigious flash space manager
    engine_audio_stop_all();
    engine_resource_reset();
    engine_gui_reset();

    engine_objects_clear_all();

    gc_collect();

    engine_display_free_depth_buffer();

    // mp_obj_t machine_module = mp_import_name(MP_QSTR_machine, mp_const_none, MP_OBJ_NEW_SMALL_INT(0));
    // mp_call_function_0(mp_load_attr(machine_module, MP_QSTR_soft_reset));
}

// ### MODULE ###

// Module functions


STATIC mp_obj_t engine_main_module_init(){
    if(is_engine_initialized == true){
        // Always do a engine reset on import since there are
        // cases when we can't catch the end of the script
        engine_main_reset();
        engine_audio_setup_playback();

        return mp_const_none;
    }
    is_engine_initialized = true;

    ENGINE_PRINTF("Engine init!\n");

    // Needs to be setup before hand since dynamicly inits array
    engine_audio_setup();
    engine_audio_setup_playback();

    engine_input_setup();
    engine_display_init();
    engine_display_send();
    engine_physics_init();
    engine_animation_init();

    #if defined(__arm__)
        adc_init();
        adc_gpio_init(BATTERY_ADC_GPIO_PIN);
        adc_select_input(BATTERY_ADC_PORT);
    #endif

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_main_module_init_obj, engine_main_module_init);


/* --- doc ---
   NAME: engine_main
   ID: engine_main
   DESC: This module needs to imported in main files using the engine. This sets up the device and engine plus resets everything every time it is imported. IMport this module anytime you want all engine nodes to be removed or when the engine should be setup
*/
STATIC const mp_rom_map_elem_t engine_main_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_main_module_init_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_main_globals, engine_main_globals_table);

const mp_obj_module_t engine_main_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_main_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_main, engine_main_user_cmodule);
