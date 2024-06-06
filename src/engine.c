#include <stdio.h>

#include "engine_object_layers.h"
#include "display/engine_display.h"
#include "display/engine_display_common.h"
#include "io/engine_io_module.h"
#include "physics/engine_physics.h"
#include "resources/engine_resource_manager.h"
#include "engine_gui.h"
#include "utility/engine_time.h"
#include "audio/engine_audio_module.h"
#include "math/engine_math.h"
#include "utility/engine_defines.h"

#include "draw/engine_display_draw.h"

#include "animation/engine_animation_module.h"

#include "py/mpstate.h"
#include "py/mphal.h"
#include "py/stream.h"

#include "py/objtype.h"
#include "py/objstr.h"

#include "engine_main.h"

// ### MODULE ###

// Module functions


// Flag to indicate that the main engine.start() loop is running. Set
// false to stop the engine after the current loop/tick ends
bool is_engine_looping = false;
bool fps_limit_disabled = true;
float engine_fps_limit_period_ms = 16.6667f;
float engine_fps_time_at_last_tick_ms = 0.0f;
float engine_fps_time_at_before_last_tick_ms = 0.0f;
float dt;


/* --- doc ---
   NAME: set_fps_limit
   ID: set_fps_limit
   DESC: Sets the FPS limit that the game engine can run at. If the game runs fast enough to reach this, engine busy waits until it is time for the next frame
   PARAM: [type=float] [name=fps] [value=any positive value]
   RETURN: None
*/
STATIC mp_obj_t engine_set_fps_limit(mp_obj_t fps_obj){
    ENGINE_INFO_PRINTF("Engine: Setting FPS");
    float fps = mp_obj_get_float(fps_obj);
    
    if(fps <= 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Engine: ERROR: Tried to set fps limit to 0 (would divide by zero) or negative value"));
    }
    
    fps_limit_disabled = false;
    engine_fps_limit_period_ms = (1.0f / fps) * 1000.0f;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_fps_limit_obj, engine_set_fps_limit);


/* --- doc ---
   NAME: disable_fps_limit
   ID: disable_fps_limit
   DESC: Disables the FPS limit. The engine will tick uncapped
   RETURN: None
*/
STATIC mp_obj_t engine_disable_fps_limit(){
    ENGINE_INFO_PRINTF("Engine: Disabling FPS limit");
    fps_limit_disabled = true;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_disable_fps_limit_obj, engine_disable_fps_limit);


/* --- doc ---
   NAME: get_running_fps
   ID: get_running_fps
   DESC: Gets the actual FPS that the game loop is running at
   RETURN: float
*/
STATIC mp_obj_t engine_get_running_fps(){
    ENGINE_INFO_PRINTF("Engine: Getting FPS");
    float period = (engine_fps_time_at_last_tick_ms - engine_fps_time_at_before_last_tick_ms) / 1000.0f;    // Seconds
    float fps = 1.0f / period;

    if(engine_math_compare_floats(fps, 0.0f) == true){
        return mp_obj_new_float(99999);
    }else{
        return mp_obj_new_float(fps);
    }
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_get_running_fps_obj, engine_get_running_fps);


// Mostly used internally when engine.stop() is called
// but exposed anyway to MicroPython
/* --- doc ---
   NAME: reset
   ID: engine_reset
   DESC: Resets internal state of engine (TODO: make sure all state is cleared, run when games end or go back to REPL or launcher)
   RETURN: None
*/
STATIC mp_obj_t engine_reset(){
    fps_limit_disabled = true;
    engine_main_reset();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_reset_obj, engine_reset);


/* --- doc ---
   NAME: end
   ID: engine_end
   DESC: Stops the main loop if it is running, otherwise resets the internal engine state right away (for the case someone is calling engine.tick() themselves)
   RETURN: None
*/
STATIC mp_obj_t engine_end(){
    ENGINE_INFO_PRINTF("Stopping engine...");

    // If the engine is looping because of engine.start(), stop it
    is_engine_looping = false;

    // Now handle the exception correctly
    mp_handle_pending(true);

    ENGINE_INFO_PRINTF("Engine stopped!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_end_obj, engine_end);


bool engine_tick(){
    // Not sure why this is needed exactly for handling ctrl-c 
    // correctly, just replicating what happens in modutime.c
    MP_THREAD_GIL_EXIT();

    dt = millis() - engine_fps_time_at_last_tick_ms;
    float dt_s = dt * 0.001f; 

    // Call the physics_tick callbacks on all physics nodes first
    engine_physics_physics_tick(dt);

    // Now that all the node callbacks were called and potentially moved
    // physics nodes around, step the physics engine another tick.
    engine_physics_tick();

    if(fps_limit_disabled || dt >= engine_fps_limit_period_ms){
        engine_fps_time_at_before_last_tick_ms = engine_fps_time_at_last_tick_ms;
        engine_fps_time_at_last_tick_ms = millis();

        ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_1, "Loop time");
        ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_1);

        // Update/grab which buttons are pressed before calling all node callbacks
        engine_io_update_pressed_buttons();

        // Goes through all animation components.
        // Do this first in case a camera is being
        // tweened or anything like that
        engine_animation_tick(dt);

        // Call every instanced node's callbacks
        engine_invoke_all_node_callbacks(dt_s);

        engine_gui_tick();

        // After every game cycle send the current active screen buffer to the display
        engine_display_send();

        // Clear the depth buffer, if needed
        engine_display_clear_depth_buffer();

        return true;
    }

    // Not sure why this is needed exactly for handling ctrl-c 
    // correctly, just replicating what happens in modutime.c
    MP_THREAD_GIL_ENTER();

    // This needs to be called for handling interrupts, but we
    // won't let it raise an exception since the engine needs
    // to end
    mp_handle_pending(false);

    // If there's an exception, make sure the engine loop ends (reset happens on `engine_main` import)
    if(MP_STATE_THREAD(mp_pending_exception) != MP_OBJ_NULL){
        engine_end();
    }

    return false;
}


/* --- doc ---
   NAME: tick
   ID: engine_tick
   DESC: Runs the main tick function of the engine. This is called in a loop when doing 'engine.start()' but can also be called manually if needed. Returns True if the tick ran and False otherwise (due to FPS limit)
   RETURN: True or False
*/
STATIC mp_obj_t engine_mp_tick(){
    return mp_obj_new_bool(engine_tick());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_mp_tick_obj, engine_mp_tick);


/* --- doc ---
   NAME: start
   ID: engine_start
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
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_module_init_obj, engine_module_init);


/* --- doc ---
   NAME: engine
   ID: engine
   DESC: Main component for controlling vital engine features
   ATTR: [type=function] [name={ref_link:set_fps_limit}]        [value=function]
   ATTR: [type=function] [name={ref_link:disable_fps_limit}]    [value=function (fps limit is disabled by default, use {ref_link:set_fps_limit} to enable it)]
   ATTR: [type=function] [name={ref_link:get_running_fps}]      [value=function]
   ATTR: [type=function] [name={ref_link:engine_tick}]          [value=function]
   ATTR: [type=function] [name={ref_link:engine_start}]         [value=function]
   ATTR: [type=function] [name={ref_link:engine_end}]           [value=function]
   ATTR: [type=function] [name={ref_link:engine_reset}]         [value=function]
*/
STATIC const mp_rom_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_fps_limit), (mp_obj_t)&engine_set_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable_fps_limit), (mp_obj_t)&engine_disable_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_running_fps), (mp_obj_t)&engine_get_running_fps_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick), (mp_obj_t)&engine_mp_tick_obj },
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
