#include "engine_save_module.h"

#include "py/obj.h"
#include "engine_main.h"
#include "utility/engine_mp.h"
#include "utility/engine_file.h"
#include "debug/debug_print.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_color.h"
#include "py/objstr.h"
#include "py/objint.h"
#include "py/objarray.h"
#include "py/binary.h"
#include "math/engine_math.h"
#include "engine_save.h"
#include "versions.h"
#include <string.h>
#include <stdlib.h>


/* --- doc ---
   NAME: set_location
   ID: engine_save_set_location
   DESC: Sets the current file location that entries will be saved to (required that this is don't before trying to save, load, or delete entries)
   PARAM:   [type=str]  [name=filepath]   [value=str]
   RETURN: None
*/
STATIC mp_obj_t engine_save_set_location(mp_obj_t location){
    ENGINE_INFO_PRINTF("EngineSave: Setting location");

    if(mp_obj_is_str(location) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path is not a string"));
    }

    GET_STR_DATA_LEN(location, str, str_len);
    engine_saving_set_file_location(str, str_len);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_save_set_location_obj, engine_save_set_location);


/* --- doc ---
   NAME: delete_location
   ID: engine_save_delete_location
   DESC: Deletes the set save file
   RETURN: None
*/
STATIC mp_obj_t engine_save_delete_location(){
    ENGINE_INFO_PRINTF("EngineSave: Deleting save location!");
    engine_saving_del_set_location();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_save_delete_location_obj, engine_save_delete_location);


/* --- doc ---
   NAME: save
   ID: engine_save_save
   DESC: Saves the value/object to the save file under the given entry name
   PARAM:   [type=str]  [name=entry_name]   [value=str]
   PARAM:   [type=str, bytearray, int, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}]  [name=value]   [value=str, int, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}]
   RETURN: None
*/
STATIC mp_obj_t engine_save(mp_obj_t entry_name_obj, mp_obj_t obj){
    ENGINE_INFO_PRINTF("EngineSave: Saving");

    GET_STR_DATA_LEN(entry_name_obj, entry_name, entry_name_len);

    if(entry_name_len == 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Tried to save entry with empty/zero length name"));
    }

    engine_saving_save_entry(entry_name, entry_name_len, obj);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_save_obj, engine_save);


/* --- doc ---
   NAME: load
   ID: engine_save_load
   DESC: Loads and returns the value/object from the save file from the given entry name. This allocates new memory which can be large depending on the data (str and bytearray). If no `default` is passed and the entry is not found, None is returned
   PARAM:   [type=str]  [name=entry_name]   [value=str]
   PARAM:   [type=any]  [name=default]      [value=any (optional, if the `entry_name` isn't found then this default value will be returned instead)]
   RETURN: str, bytearray, int, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}, or None
*/
STATIC mp_obj_t engine_save_load(size_t n_args, const mp_obj_t *args){
    mp_obj_t entry_name_obj = args[0];
    mp_obj_t default_value_obj = mp_const_none;

    if(n_args == 2){
        default_value_obj = args[1];
    }

    GET_STR_DATA_LEN(entry_name_obj, entry_name, entry_name_len);

    if(entry_name_len == 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Tried to load entry with empty/zero length name"));
    }

    mp_obj_t entry = engine_saving_load_entry(entry_name, entry_name_len);

    if(entry == mp_const_none){
        return default_value_obj;
    }else{
        return entry;
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_save_load_obj, 1, 2, engine_save_load);


/* --- doc ---
   NAME: delete
   ID: engine_save_delete
   DESC: Deletes the save entry inside the save file for the given entry name
   PARAM:   [type=str]  [name=entry_name]   [value=str]
   RETURN: None
*/
STATIC mp_obj_t engine_save_delete(mp_obj_t entry_name_obj){
    GET_STR_DATA_LEN(entry_name_obj, entry_name, entry_name_len);

    if(entry_name_len == 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Tried to delete entry with empty/zero length name"));
    }

    engine_saving_delete_entry(entry_name, entry_name_len);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_save_delete_obj, engine_save_delete);


STATIC mp_obj_t engine_save_module_init(){
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_save_module_init_obj, engine_save_module_init);    


/* --- doc ---
   NAME: engine_save
   ID: engine_save
   DESC: Module for saving and loading data
   ATTR:   [type=function]    [name={ref_link:engine_save_set_location}]        [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_delete_location}]     [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_save}]                [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_load}]                [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_delete}]              [value=function]
*/
STATIC const mp_rom_map_elem_t engine_save_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_save) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_save_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_save), (mp_obj_t)&engine_save_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_load), (mp_obj_t)&engine_save_load_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_delete), (mp_obj_t)&engine_save_delete_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_delete_location), (mp_obj_t)&engine_save_delete_location_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_location), (mp_obj_t)&engine_save_set_location_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT(mp_module_engine_save_globals, engine_save_globals_table);

const mp_obj_module_t engine_save_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_save_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_save, engine_save_user_cmodule);
