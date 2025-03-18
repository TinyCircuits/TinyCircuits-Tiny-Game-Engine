#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "engine_main.h"
#include "engine.h"
#include "math/engine_math.h"

#include "engine_object_layers.h"
#include "resources/engine_resource_manager.h"
#include "audio/engine_audio_module.h"
#include "io/engine_io_module.h"
#include "save/engine_save_module.h"
#include "time/engine_rtc.h"
#include "display/engine_display.h"
#include "display/engine_display_common.h"
#include "physics/engine_physics.h"
#include "animation/engine_animation_module.h"
#include "engine_gui.h"
#include "fault/engine_fault.h"
#include "link/engine_link_module.h"
#include "py/mpstate.h"

#if defined(__EMSCRIPTEN__)

#elif defined(__unix__)
    #include <dirent.h>
#elif defined(__arm__)
    
#endif


char filesystem_root[FILESYSTEM_ROOT_MAX_LEN];
bool is_engine_initialized = false;


mp_obj_str_t settings_dir_location = {
    .base.type = &mp_type_str,
    .hash = 0,
    .data = (byte[]){"/system"},
    .len = 7,
};

mp_obj_str_t settings_file_location = {
    .base.type = &mp_type_str,
    .hash = 0,
    .data = (byte[]){"/system/settings.txt"},
    .len = 20,
};

void engine_main_raise_if_not_initialized(){
    if(is_engine_initialized == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineMain: ERROR: `import engine_main` needs to be done at least once at the very start!"));
    }
}


void engine_main_settings_write(float volume, float brightness){
    engine_file_open_create_write(0, &settings_file_location);

    char buffer[32];

    int size = snprintf(buffer, 32, "volume=%0.2f\n", (double)volume);
    engine_file_write(0, buffer, size);

    size = snprintf(buffer, 32, "brightness=%0.2f", (double)brightness);
    engine_file_write(0, buffer, size);

    engine_file_close(0);
}


void engine_main_settings_read(){
    // Default values
    float volume = 1.0f;
    float brightness = 1.0f;

    engine_file_open_read(0, &settings_file_location);
    uint32_t file_size = engine_file_size(0);
    
    // Buffer to hold read characters
    char line_buffer[32] = {0};
    uint8_t line_buffer_cursor = 0;
    char character = '\0';
    uint8_t equals_index = 0;
    uint8_t line_number = 1;
    uint32_t total_read_amount = 0;
    
    while(true){
        // Accumlate one character at a time into the buffer
        uint8_t read_amount = engine_file_read(0, &character, 1);
        total_read_amount += read_amount;

        // Ignore spaces but accumulate everything else
        if(character != ' '){
            line_buffer[line_buffer_cursor] = character;

            // Need to track the seperator (equals sign)
            if(character == '='){
                equals_index = line_buffer_cursor;
            }

            line_buffer_cursor++;
        }

        // If we're at the end of a line/file, parse the buffer
        if(character == '\n' || total_read_amount == file_size){
            if(equals_index == 0){
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineMain: ERROR: Could not find '=' sign on line %d of 'system/settings.txt' file!"), line_number);
            }

            if(strncmp("volume", line_buffer, equals_index) == 0){
                volume = strtof(line_buffer+equals_index+1, NULL);
            }else if(strncmp("brightness", line_buffer, equals_index) == 0){
                brightness = strtof(line_buffer+equals_index+1, NULL);
            }else{
                ENGINE_WARNING_PRINTF("EngineMain: WARNING: Could not parse line in 'system/settings.txt' file! Unknown value...");
            }

            // Reset and increment these every line
            line_buffer_cursor = 0;
            character = 0;
            equals_index = 0;
            memset(line_buffer, 0, sizeof(line_buffer));
            line_number++;
        }

        // Break the loop when we reach the end of the file
        if(total_read_amount == file_size){
            break;
        }
    }

    // No matter what, close the file in this index
    engine_file_close(0);

    // Clamp and re-write values if out of bounds
    if((volume < 0.0f || volume > 1.0f) || (brightness < 0.05f || brightness > 1.0f)){
        volume = engine_math_clamp(volume, 0.0f, 1.0f);
        brightness = engine_math_clamp(brightness, 0.05f, 1.0f);

        engine_main_settings_write(volume, brightness);
    }

    ENGINE_PRINTF("Volume: %0.2f, Brightness: %0.2f\n", (double)volume, (double)brightness);

    // Set the master volume and brightness in the other engine modules
    engine_audio_apply_master_volume((float)volume);
    engine_display_apply_brightness((float)brightness);
}


void engine_main_handle_settings(){
    ENGINE_PRINTF("Settings location: %s\n", settings_file_location.data);

    // Create settings file if it does not exist, otherwise,
    // parse the file
    if(!engine_file_exists(&settings_file_location)){
        // Only if the system folder doesn't exist do we create it
        if(!engine_file_exists(&settings_dir_location)) engine_file_mkdir(&settings_dir_location);
        engine_main_settings_write(1.0f, 1.0f);
    }else{
        engine_main_settings_read();
    }
}


void engine_main_reset(){
    ENGINE_PRINTF("EngineMain: Resetting engine...\n");

    // Probably should reset the processor clock speed to base 150MHz (depending on platform): TODO - engine_set_freq(150 * 1000 * 1000);

    engine_display_reset_fills();       // Always reset screen background fills to no texture and black color
    engine_link_module_reset();         // Reset callbacks to None and stop link if started
    engine_io_reset();                  // Reset certain flags like if the indicator is overriden by the game and is not showing battery level
    engine_audio_reset();               // Reset game volume and stop all channels from playing audio
    engine_resource_reset();            // Reset contigious flash space manager (TODO: should implement some wear-leveling)
    engine_gui_reset();                 // Reset flags for overriding input to GUI system
    engine_objects_clear_all();         // Clear all nodes so that they get collected and not drawn anymores
    engine_display_free_depth_buffer(); // If the depth buffer was allocated, free it

    gc_collect();                       // Collect all MicroPython memory

    engine_io_setup();                  // IO setup should be called first so that RGB and motor rumble PWM get their required slices for their GPIO
    engine_audio_setup_playback();      // This should be called after IO setup so that all required slices for other IO are taken first and then callback for audio playback can be put on any unused slice
    engine_display_init_framebuffers(); // Always recreate the MicroPython framebuffers after they were collected (not the actual memory where pixels are stored, just wrappers over that)
}


void engine_main_one_time_setup(){
    ENGINE_PRINTF("Engine init!\n");

    engine_io_setup();
    engine_io_battery_monitor_setup();

    engine_resource_init();

    // Init display first
    engine_display_init();
    engine_display_init_framebuffers();
    engine_display_send();
    engine_display_clear();

    // Setup fault handlers after screen is setup since
    // we need the screen to output the fault
    engine_fault_handling_register();

    // Needs to be setup before hand since dynamicly inits array
    engine_audio_setup_playback();
    engine_audio_setup();

    // One time setups for 
    engine_physics_init();
    engine_animation_init();
    engine_rtc_init();
}

// ### MODULE ###

// Module functions


static mp_obj_t engine_main_module_init(){
    // If the engine has not been initialzed yet, get the filesystem root
    if(!is_engine_initialized){
        #if defined(__EMSCRIPTEN__) || defined(__arm__)
            // On hardware or in the browser, it's easy, it is always '/'
            filesystem_root[0] = '/';
            filesystem_root[1] = '\0';
        #elif defined(__unix__)
            // On Linux, assume the user knows to execute while in `filesystem` folder
            if(getcwd(filesystem_root, sizeof(filesystem_root)) == NULL){
                filesystem_root[0] = '\0';
            }
        #endif

        ENGINE_PRINTF("Filesystem root: %s\n", filesystem_root);
    }

    // Handle settings setup
    engine_main_handle_settings();

    if(is_engine_initialized == true){
        // Always do a engine reset on import since there are
        // cases when we can't catch the end of the script
        engine_main_reset();
        
        // On subsequent resets, anything allocated with m_tracked_buffers
        // will need to be restored since they are erased in soft resets
        return mp_const_none;
    }

    is_engine_initialized = true;
    engine_main_one_time_setup();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_main_module_init_obj, engine_main_module_init);


/* --- doc ---
   NAME: engine_main
   ID: engine_main
   DESC: This module needs to imported in main files using the engine. This sets up the device and engine plus resets everything every time it is imported. Import this module anytime you want all engine nodes to be removed or when the engine should be setup
*/
static const mp_rom_map_elem_t engine_main_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_main_module_init_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_main_globals, engine_main_globals_table);

const mp_obj_module_t engine_main_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_main_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_main, engine_main_user_cmodule);
