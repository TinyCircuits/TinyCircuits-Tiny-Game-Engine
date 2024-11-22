#include <stdio.h>
#include <string.h>

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
#include "link/engine_link_module.h"

#include "draw/engine_display_draw.h"

#include "animation/engine_animation_module.h"

#include "py/mpstate.h"
#include "py/mphal.h"
#include "py/stream.h"

#include "py/objtype.h"
#include "py/objstr.h"

#include "engine_main.h"


#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>

    // This is called by the MicroPython VM hook so that
    // new serial data can be retrieved from the web page
    // while just MicroPython is running, not just the engine
    EM_JS(void, new_hook, (), {
        self.get_serial();
    });
#elif defined(__arm__)
    #include "hardware/clocks.h"
    #include "hardware/pll.h"
    #include "hardware/structs/rosc.h"
    #include "hardware/structs/scb.h"
    #include "hardware/structs/syscfg.h"
    #include "hardware/watchdog.h"
    #include "hardware/xosc.h"
    #include "py/mphal.h"
#endif


// ### MODULE ###

// Module functions


// Flag to indicate that the main engine.start() loop is running. Set
// false to stop the engine after the current loop/tick ends
bool is_engine_looping = false;
bool fps_limit_disabled = true;
float engine_fps_limit_period_ms = 1;  // limit disabled initially anyway
uint32_t engine_fps_time_at_last_tick_ms = MILLIS_NULL;
uint32_t engine_fps_time_at_before_last_tick_ms = MILLIS_NULL;


float engine_get_fps_limit_ms(){
    return engine_fps_limit_period_ms;
}


void engine_set_freq(uint32_t hz){
    #if defined(__arm__)
        if(!set_sys_clock_khz(hz / 1000, false)){
            mp_raise_ValueError(MP_ERROR_TEXT("cannot change frequency"));
        }
        engine_audio_adjust_playback_with_freq(hz);
    #endif
}


/* --- doc ---
   NAME: fps_limit
   ID: fps_limit
   DESC: Gets or sets the FPS limit that the game engine can run at. If the game runs fast enough to reach this, engine busy waits until it is time for the next frame. Infinity (math.inf) means a disabled FPS limit.
   PARAM: [type=float (optional)] [name=fps] [value=a positive FPS value]
   RETURN: None or float
*/
static mp_obj_t engine_fps_limit(size_t n_args, const mp_obj_t *args){
    if(n_args == 0){
        if (fps_limit_disabled){
            return mp_obj_new_float(INFINITY);
        }
        return mp_obj_new_float(1000.0f / engine_get_fps_limit_ms());
    }

    ENGINE_INFO_PRINTF("Engine: Setting FPS");
    float fps = mp_obj_get_float(args[0]);

    if(fps <= 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Engine: ERROR: Tried to set fps limit to 0 (would divide by zero) or negative value"));
    }

    if(fps == INFINITY){
        fps_limit_disabled = true;
    }else{
        fps_limit_disabled = false;
        engine_fps_limit_period_ms = 1000.0f / fps;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_fps_limit_obj, 0, 1, engine_fps_limit);


/* --- doc ---
   NAME: disable_fps_limit
   ID: disable_fps_limit
   DESC: Disables the FPS limit. The engine will tick uncapped
   RETURN: None
*/
static mp_obj_t engine_disable_fps_limit(){
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
static mp_obj_t engine_get_running_fps(){
    ENGINE_INFO_PRINTF("Engine: Getting FPS");
    if(engine_fps_time_at_before_last_tick_ms == MILLIS_NULL){
        return mp_obj_new_float(99999);
    }
    float period = millis_diff(engine_fps_time_at_last_tick_ms, engine_fps_time_at_before_last_tick_ms) / 1000.0f;    // Seconds
    float fps = 1.0f / period;

    if(engine_math_compare_floats(fps, 0.0f) == true){
        return mp_obj_new_float(99999);
    }else{
        return mp_obj_new_float(fps);
    }
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_get_running_fps_obj, engine_get_running_fps);


/* --- doc ---
   NAME: reset
   ID: engine_reset
   DESC: machine.reset() but works across all platforms. Performs a complete hard reset of the device by default.
   PARAM: [type=bool]   [name=soft_reset]   [value=True or False (default: False)]
   RETURN: None
*/
static mp_obj_t engine_reset(size_t n_args, const mp_obj_t *args){
    // Set default reset as not a soft reset and see if any arguments set it
    bool soft_reset = false;

    if(n_args == 1){
        if(mp_obj_is_bool(args[0])){
            soft_reset = mp_obj_get_int(args[0]);
        }else{
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("Engine: ERROR: Expected `int` got %s"), mp_obj_get_type_str(args[0]));
        }
    }

    // Do the reset depending on the platform
    #if defined(__EMSCRIPTEN__)
        exit(93);
        (void)soft_reset;
    #elif defined(__unix__)
        exit(93);
        (void)soft_reset;
    #else
        mp_obj_t machine_module = mp_import_name(MP_QSTR_machine, mp_const_none, MP_OBJ_NEW_SMALL_INT(0));

        if(soft_reset){
            mp_call_function_0(mp_load_attr(machine_module, MP_QSTR_soft_reset));
        }else{
            mp_call_function_0(mp_load_attr(machine_module, MP_QSTR_reset));
        }
    #endif

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_reset_obj, 0, 1, engine_reset);


/* --- doc ---
   NAME: end
   ID: engine_end
   DESC: Stops the main loop if it is running, otherwise resets the internal engine state right away (for the case someone is calling engine.tick() themselves)
   RETURN: None
*/
static mp_obj_t engine_end(){
    ENGINE_INFO_PRINTF("Stopping engine...");

    // If the engine is looping because of engine.start(), stop it
    is_engine_looping = false;

    // Now handle the exception correctly
    mp_handle_pending(true);

    ENGINE_INFO_PRINTF("Engine stopped!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_end_obj, engine_end);



/* --- doc ---
   NAME: time_to_next_tick
   ID: engine_time_to_next_tick
   DESC: Returns the time in millis until the next desired engine tick. If the FPS limit is disabled, returns 0. If the desired tick time has already passed, returns 0. This might be useful when creating an asynchronous main loop.
   RETURN: int
*/
static mp_obj_t engine_mp_time_to_next_tick(){
    int32_t time_to_next_tick;
    if(fps_limit_disabled || engine_fps_time_at_last_tick_ms == MILLIS_NULL){
        time_to_next_tick = 0;
    }else{
        int32_t dt_ms = millis_diff(millis(), engine_fps_time_at_last_tick_ms);
        time_to_next_tick = (int32_t)(engine_fps_limit_period_ms + 0.5f) - dt_ms;
        if(time_to_next_tick < 0){
            time_to_next_tick = 0;
        }
    }
    return mp_obj_new_int(time_to_next_tick);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_time_to_next_tick_obj, engine_mp_time_to_next_tick);


bool engine_tick(){
    // Run this as often as possible
    engine_link_module_task();

    bool ticked = false;

    // Not sure why this is needed exactly for handling ctrl-c
    // correctly, just replicating what happens in modutime.c
    MP_THREAD_GIL_EXIT();

    uint32_t now = millis();
    float dt_ms;
    if(engine_fps_time_at_last_tick_ms == MILLIS_NULL){
        dt_ms = engine_fps_limit_period_ms;
    }else{
        dt_ms = (float)millis_diff(now, engine_fps_time_at_last_tick_ms);
    }

    // Now that all the node callbacks were called and potentially moved
    // physics nodes around, step the physics engine another tick.
    engine_physics_tick();

    if(fps_limit_disabled || dt_ms >= engine_fps_limit_period_ms){
        engine_fps_time_at_before_last_tick_ms = engine_fps_time_at_last_tick_ms;
        engine_fps_time_at_last_tick_ms = now;

        ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_1, "Loop time");
        ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_1);

        float dt_s = dt_ms * 0.001f;

        // Update/grab which buttons are pressed before calling all node callbacks
        engine_io_tick();

        // Goes through all animation components.
        // Do this first in case a camera is being
        // tweened or anything like that
        engine_animation_tick(dt_ms);

        // Call every instanced node's callbacks
        engine_invoke_all_node_tick_callbacks(dt_s);
        engine_objects_clear_deletable();                       // Remove any nodes marked for deletion before rendering
        engine_invoke_all_node_draw_callbacks();

        engine_gui_tick();

        // After every game cycle send the current active screen buffer to the display
        engine_display_send();

        // Clear the depth buffer, if needed
        engine_display_clear_depth_buffer();

        ticked = true;
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

    return ticked;
}


/* --- doc ---
   NAME: dt
   ID: engine_dt
   DESC: Returns the time, in seconds, since the last {ref_link:engine_tick}
   RETURN: True or False
*/
static mp_obj_t engine_mp_dt(){
    return mp_obj_new_float(millis_diff(millis(), engine_fps_time_at_last_tick_ms) * 0.001f);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_mp_dt_obj, engine_mp_dt);


/* --- doc ---
   NAME: tick
   ID: engine_tick
   DESC: Runs the main tick function of the engine. This is called in a loop when doing 'engine.start()' but can also be called manually if needed. Returns True if the tick ran and False otherwise (due to FPS limit)
   RETURN: True or False
*/
static mp_obj_t engine_mp_tick(){
    return mp_obj_new_bool(engine_tick());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_mp_tick_obj, engine_mp_tick);


/* --- doc ---
   NAME: start
   ID: engine_start
   DESC: Starts the main engine loop to start calling 'engine.tick()'
   RETURN: None
*/
static mp_obj_t engine_start(){
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


/* --- doc ---
   NAME: freq
   ID: engine_freq
   DESC: Gets or sets the processor frequency (use this in place of machine.freq(Hz))
   PARAM: [type=int] [name=hz] [value=any positive integer | optional]
   RETURN: None or int
*/
static mp_obj_t engine_freq(size_t n_args, const mp_obj_t *args){
    if(n_args == 1){
        engine_set_freq(mp_obj_get_int(args[0]));
    }else{
        #if defined(__arm__)
            return mp_obj_new_int(mp_hal_get_cpu_freq());
        #endif
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_freq_obj, 0, 1, engine_freq);


static mp_obj_t engine_root_dir(){
    return mp_obj_new_str(filesystem_root, strlen(filesystem_root));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_root_dir_obj, engine_root_dir);


static mp_obj_t engine_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_module_init_obj, engine_module_init);


/* --- doc ---
   NAME: engine
   ID: engine
   DESC: Main component for controlling vital engine features
   ATTR: [type=function] [name={ref_link:fps_limit}]                   [value=getter/setter function]
   ATTR: [type=function] [name={ref_link:disable_fps_limit}]           [value=function (fps limit is disabled by default, use {ref_link:fps_limit} to enable it)]
   ATTR: [type=function] [name={ref_link:get_running_fps}]             [value=function]
   ATTR: [type=function] [name={ref_link:engine_time_to_next_tick}]    [value=function]
   ATTR: [type=function] [name={ref_link:engine_tick}]                 [value=function]
   ATTR: [type=function] [name={ref_link:engine_dt}]                   [value=function]
   ATTR: [type=function] [name={ref_link:engine_start}]                [value=function]
   ATTR: [type=function] [name={ref_link:engine_end}]                  [value=function]
   ATTR: [type=function] [name={ref_link:engine_reset}]                [value=function]
   ATTR: [type=function] [name={ref_link:engine_freq}]                 [value=function]
*/
static const mp_rom_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_fps_limit), (mp_obj_t)&engine_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable_fps_limit), (mp_obj_t)&engine_disable_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_running_fps), (mp_obj_t)&engine_get_running_fps_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_time_to_next_tick), (mp_obj_t)&engine_time_to_next_tick_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tick), (mp_obj_t)&engine_mp_tick_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dt), (mp_obj_t)&engine_mp_dt_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_end), (mp_obj_t)&engine_end_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset), (mp_obj_t)&engine_reset_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_freq), (mp_obj_t)&engine_freq_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_root_dir), (mp_obj_t)&engine_root_dir_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);
