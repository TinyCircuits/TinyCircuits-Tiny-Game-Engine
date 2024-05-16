#include "engine_save_module.h"

#include "py/obj.h"
#include "engine_main.h"
#include "utility/engine_file.h"
#include "debug/debug_print.h"
#include <string.h>


size_t current_location_len = 0;    // Current length of the saving location (less than `current_location_len_max`)      
char current_location[256];

uint8_t count = 32;


STATIC mp_obj_t engine_set_location(mp_obj_t location){
    ENGINE_INFO_PRINTF("EngineSave: Setting location");

    if(mp_obj_is_str(location) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path is not a string"));
    }

    const char *location_str = mp_obj_str_get_data(location, &current_location_len);

    // Cannot be larger than this since `temp-` takes up 5 characters
    if(current_location_len > 250){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path too long (max length: 256)"));
    }

    // Copy name to 5 chars after start to fit `temp-`
    memcpy(current_location+5, location_str, current_location_len);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_location_obj, engine_set_location);


// void engine_save_start(){
//     current_location[0] = 't';
//     current_location[1] = 'e';
//     current_location[2] = 'm';
//     current_location[3] = 'p';
//     current_location[4] = '-';
//     engine_file_open(current_location);
// }


// void engine_save_end(){
//     engine_file_write((uint8_t[]){count}, 1);
//     engine_file_close();
//     engine_file_remove(current_location+5);
//     engine_file_rename(current_location, current_location+5);
//     count++;
// }


STATIC mp_obj_t engine_save(mp_obj_t save_name, mp_obj_t obj){
    ENGINE_INFO_PRINTF("EngineSave: Saving");

    if(mp_obj_is_str(save_name) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: save name is not a string, cannot save!"));
    }

    if(current_location_len == 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path not set, cannot save!"));
    }

    // engine_save_start();
    // engine_save_end();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_save_obj, engine_save);


STATIC mp_obj_t engine_save_module_init(){
    current_location_len = 0;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_save_module_init_obj, engine_save_module_init);    


/* --- doc ---
   NAME: engine_save
   ID: engine_save
   DESC: Module for saving and loading data
*/
STATIC const mp_rom_map_elem_t engine_save_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_save) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_save_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_save), (mp_obj_t)&engine_save_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_location), (mp_obj_t)&engine_set_location_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT(mp_module_engine_save_globals, engine_save_globals_table);

const mp_obj_module_t engine_save_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_save_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_save, engine_save_user_cmodule);
