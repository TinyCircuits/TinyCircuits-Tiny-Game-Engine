#include <stdio.h>

#include "engine_object_layers.h"
#include "display/engine_display.h"
#include "input/engine_input_module.h"
#include "physics/engine_physics.h"
#include "resources/engine_resource_manager.h"
#include "engine_cameras.h"
#include "utility/engine_time.h"
#include "audio/engine_audio_module.h"

#include "display/engine_display.h"
#include "draw/engine_display_draw.h"

#include "py/mpstate.h"
#include "py/mphal.h"
#include "py/stream.h"
#include "py/gc.h"

// ### MODULE ###

// Module functions


// Flag to indicate that the main engine.start() loop is running. Set
// false to stop the engine after the current loop/tick ends
bool is_engine_looping = false;
bool is_engine_initialized = false;
float engine_fps_limit_period_ms = 0.0f;
float engine_fps_time_at_last_tick_ms = 0.0f;
float engine_fps_time_at_before_last_tick_ms = 0.0f;


/* --- doc ---
   NAME: set_fps_limit
   DESC: Sets the FPS limit that the game engine can run at. If the game runs fast enough to reach this, engine busy waits until it is time for the next frame
   PARAM: [type=float] [name=fps] [value=any positive value]
   RETURN: None
*/
STATIC mp_obj_t engine_set_fps_limit(mp_obj_t fps_obj){
    ENGINE_INFO_PRINTF("Engine: Setting FPS");
    float fps = mp_obj_get_float(fps_obj);
    
    if(fps < 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Engine: ERROR: Tried to set fps limit to negative value"));
    }
    
    engine_fps_limit_period_ms = (1.0f / fps) * 1000.0f;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_fps_limit_obj, engine_set_fps_limit);


/* --- doc ---
   NAME: get_running_fps
   DESC: Gets the actual FPS that the game loop is running at
   RETURN: float
*/
STATIC mp_obj_t engine_get_running_fps(){
    ENGINE_INFO_PRINTF("Engine: Getting FPS");
    return mp_obj_new_float((mp_float_t)(1.0f / ((engine_fps_time_at_last_tick_ms - engine_fps_time_at_before_last_tick_ms)/1000.0f)));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_get_running_fps_obj, engine_get_running_fps);


// Mostly used internally when engine.stop() is called
// but exposed anyway to MicroPython
/* --- doc ---
   NAME: reset
   DESC: Resets internal state of engine (TODO: make sure all state is cleared, run when games end or go back to REPL or launcher)
   RETURN: None
*/
STATIC mp_obj_t engine_reset(){
    ENGINE_FORCE_PRINTF("Resetting engine...");

    engine_resource_reset();

    // engine_audio_stop();
    // engine_camera_clear_all();
    // engine_physics_clear_all();
    // engine_objects_clear_all();
    // engine_resource_reset();

    // gc_sweep_all();
    // gc_collect();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_reset_obj, engine_reset);


/* --- doc ---
   NAME: end
   DESC: Stops the main loop if it is running, otherwise resets the internal engine state right away (for the case someone is calling engine.tick() themselves)
   RETURN: None
*/
STATIC mp_obj_t engine_end(){
    ENGINE_INFO_PRINTF("Stopping engine...");

    // If the engine is looping because of engine.start(), stop it
    is_engine_looping = false;

    engine_reset();

    // Now handle the exception correctly since the engine is reset
    mp_handle_pending(true);

    ENGINE_INFO_PRINTF("Engine stopped!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_end_obj, engine_end);


/* --- doc ---
   NAME: tick
   DESC: Runs the main tick function of the engine. This is called in a loop when doing 'engine.start()' but can also be called manually if needed
   RETURN: None
*/
STATIC mp_obj_t engine_tick(){
    // Not sure why this is needed exactly for handling ctrl-c 
    // correctly, just replicating what happens in modutime.c
    MP_THREAD_GIL_EXIT();

    if(millis() - engine_fps_time_at_last_tick_ms >= engine_fps_limit_period_ms){
        engine_fps_time_at_before_last_tick_ms = engine_fps_time_at_last_tick_ms;
        engine_fps_time_at_last_tick_ms = millis();

        ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_1, "Loop time");
        ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_1);

        // Update/grab which buttons are pressed before calling all node callbacks
        engine_input_update_pressed_buttons();

        // Call every instanced node's callbacks
        engine_invoke_all_node_callbacks();

        // Now that all the node callbacks were called and potentially moved
        // physics nodes around, step the physics engine another tick.
        // NOTE: Before each nodes' callbacks are called the position
        // from the physics engine is synced to the engine node. After
        // all the callbacks for the physics nodes are done, the positions
        // from the engine node are synced back to the physics body
        engine_physics_tick();

        // After every game cycle send the current active screen buffer to the display
        engine_display_send();
    }

    // Not sure why this is needed exactly for handling ctrl-c 
    // correctly, just replicating what happens in modutime.c
    MP_THREAD_GIL_ENTER();

    // This needs to be called for handling interrupts, but we
    // won't let it raise an exception since the engine needs
    // to end
    mp_handle_pending(false);

    // If there's an exception, make sure the engine loop ends (reset happens on `engine` import)
    if(MP_STATE_THREAD(mp_pending_exception) != MP_OBJ_NULL){
        engine_end();
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_tick_obj, engine_tick);


/* --- doc ---
   NAME: start
   DESC: Starts the main engine loop to start calling 'engine.tick()'
   RETURN: None
*/
STATIC mp_obj_t engine_start(){
    ENGINE_INFO_PRINTF("Engine loop starting...");

    is_engine_looping = true;
    while(is_engine_looping){
        
        engine_tick();
        // // See ports/rp2/mphalport.h, ports/rp2/mphalport.c, py/mphal.h, shared/runtime/sys_stdio_mphal.c
        // // Can get chars from REPL and do stuff with them!
        // if(mp_hal_stdio_poll(MP_STREAM_POLL_RD)){
        //     int received = mp_hal_stdin_rx_chr();

        //     if(received == 3){
        //         break;
        //     }
        // }
    }


    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_start_obj, engine_start);


STATIC mp_obj_t engine_module_init(){
    if(is_engine_initialized == true){
        // Always do a engine reset on import since there are
        // cases when we can't catch the end of the script
        engine_reset();

        return mp_const_none;
    }
    is_engine_initialized = true;

    ENGINE_FORCE_PRINTF("Engine init!");

    // Needs to be setup before hand since dynamicly inits array.
    // Should make sure this doesn't happen more than once per
    // lifetime. TODO
    // engine_audio_setup();
    engine_input_setup();
    engine_display_init();
    engine_display_send();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_module_init_obj, engine_module_init);


/* --- doc ---
   NAME: engine
   DESC: Main component for controlling vital engine features
   ATTR: [type=function] [name={ref_link:set_fps_limit}]        [value=function]
   ATTR: [type=function] [name={ref_link:get_running_fps}]      [value=function]
   ATTR: [type=function] [name={ref_link:tick}]                 [value=function]
   ATTR: [type=function] [name={ref_link:start}]                [value=function]
   ATTR: [type=function] [name={ref_link:end}]                  [value=function]
   ATTR: [type=function] [name={ref_link:reset}]                [value=function]
*/
STATIC const mp_rom_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_fps_limit), (mp_obj_t)&engine_set_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_running_fps), (mp_obj_t)&engine_get_running_fps_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick), (mp_obj_t)&engine_tick_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_end), (mp_obj_t)&engine_end_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset), (mp_obj_t)&engine_reset_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);
