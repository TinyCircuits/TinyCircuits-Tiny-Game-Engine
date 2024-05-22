#include "engine_save_module.h"

#include "py/obj.h"
#include "engine_main.h"
#include "utility/engine_mp.h"
#include "utility/engine_file.h"
#include "debug/debug_print.h"
#include "py/objstr.h"
#include "py/objint.h"
#include <string.h>
#include <stdlib.h>

#define SAVE_LOCATION_LENGTH_MAX 256
#define SAVE_ENTRY_LENGTH_MAX 256

mp_obj_str_t current_location = {
    .base.type = &mp_type_str,
    .hash = 0,
    .data = (byte[SAVE_LOCATION_LENGTH_MAX]){},
    .len = 0,
};

// Need a duplicate for renaming
mp_obj_str_t temporary_location = {
    .base.type = &mp_type_str,
    .hash = 0,
    .data = (byte[SAVE_LOCATION_LENGTH_MAX]){},
    .len = 0,
};

// Buffer used for storing items from files. Example,
// if a float is being restored then it's ASCII
// representation will be stored here
char buffer[SAVE_ENTRY_LENGTH_MAX];

uint32_t reading_file_size = 0;

enum entry_types {NONE=0, STR=1, INT=2, FLT=3};


STATIC mp_obj_t engine_set_location(mp_obj_t location){
    ENGINE_INFO_PRINTF("EngineSave: Setting location");

    if(mp_obj_is_str(location) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path is not a string"));
    }

    const char *location_str = mp_obj_str_get_data(location, &current_location.len);

    // Cannot be larger than this since `temp-` takes up 5 characters
    if(current_location.len > SAVE_LOCATION_LENGTH_MAX-5){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path too long (max length: 250)"));
    }

    // Set these equal to each other, for now
    temporary_location.len = current_location.len;

    // Copy base files name to current and temporary file names
    memcpy(current_location.data, location_str, current_location.len);
    memcpy(temporary_location.data, location_str, current_location.len);

    // Append `-temp` to end of temporary filename and increase length to account for it
    memcpy(temporary_location.data + temporary_location.len, "-temp", 5);
    temporary_location.len += 5;

    // If the file we are going to read from does not exist already, create it
    if(engine_file_exists(&current_location) == false){
        engine_file_open_create_write(0, &current_location);
        engine_file_close(0);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_location_obj, engine_set_location);


void engine_save_start(){
    // Open the the file to read from
    engine_file_open_read(0, &current_location);

    reading_file_size = engine_file_size(0);

    // Open the file to write to (temporary)
    engine_file_open_create_write(1, &temporary_location);

    // Seek to start of files, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
    engine_file_seek(1, 0, MP_SEEK_SET);
}


void engine_save_end(){
    // Close the reading and writing files
    engine_file_close(0);
    engine_file_close(1);

    // Remove the file we read data from and copied
    engine_file_remove(&current_location);

    // Rename temporary file to what we just deleted
    engine_file_rename(&temporary_location, &current_location);
}


void engine_load_start(){
    // Open the the file to read from
    engine_file_open_read(0, &current_location);

    reading_file_size = engine_file_size(0);

    // Seek to start of file, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
}


void engine_load_end(){
    // Close the reading and writing files
    engine_file_close(0);
}


// TODO: check lengths of data when saving!
void engine_save_store_obj(uint8_t file_index, mp_obj_t obj){
    if(mp_obj_is_str(obj)){
        const char* obj_str = mp_obj_str_get_str(obj);
        GET_STR_LEN(obj, obj_str_len);

        engine_file_write(file_index, "TYPE=STR\n", 9);
        engine_file_write(file_index, obj_str, obj_str_len);
    }else if(mp_obj_is_int(obj)){
        engine_file_write(file_index, "TYPE=INT\n", 9);
        uint32_t len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%ld", mp_obj_get_int(obj));
        engine_file_write(file_index, buffer, len);
    }else if(mp_obj_is_float(obj)){
        engine_file_write(file_index, "TYPE=FLT\n", 9);
        uint32_t len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f", mp_obj_get_float(obj));
        engine_file_write(file_index, buffer, len);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Saving this type of object is not implemented!"));
    }

    // After the type, write the newline
    engine_file_write(1, "\n", 1);
}


STATIC mp_obj_t engine_save(mp_obj_t entry_name_obj, mp_obj_t obj){
    ENGINE_INFO_PRINTF("EngineSave: Saving");

    if(mp_obj_is_str(entry_name_obj) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: entry name is not a string, cannot save!"));
    }

    // Open read and write files (create read file too if needed)
    engine_save_start();

    // Setup for finding line to save new data at
    const char *entry_name = mp_obj_str_get_str(entry_name_obj);
    GET_STR_LEN(entry_name_obj, entry_name_len);
    uint8_t entry_name_index = 0;

    char character = ' ';
    bool entry_already_existed = false;

    // While not at end of file, read file, check for name, and copy
    while(engine_file_read(0, &character, 1) != 0){
        // Copy to temporary save file
        engine_file_write(1, &character, 1);

        // If we find a character that's in the save name,
        // increase index into save name and keep checking
        // else reset if miss a character
        if(character == entry_name[entry_name_index]){
            entry_name_index++;
        }else{
            entry_name_index = 0;
        }

        // If we found all characters in the entry name, 
        // stop the loop then write the object out
        if(entry_name_index == entry_name_len){
            entry_already_existed = true;
            break;
        }
    }

    // If the name was not found, add it, otherwise
    // just do newline since consumed in the search
    if(entry_already_existed == false){
        engine_file_write(1, "NAME=", 5);
        engine_file_write(1, entry_name, entry_name_len);
        engine_file_write(1, "\n", 1);
    }else{
        engine_file_write(1, "\n", 1);
    }

    // Save the object to the file
    engine_save_store_obj(1, obj);

    // In the reading file, skip the old overwritten
    // data and copy the rest of the file to temporary
    uint32_t position = engine_file_seek_until(0, "NAME=", 5);

    // Make sure we aren't really at the end of the
    // file, if not, go back to before `NAME=` and copy
    if(position != reading_file_size){
        engine_file_seek(0, -5, MP_SEEK_CUR);
    }

    // Copy the rest of the file
    while(engine_file_read(0, &character, 1) != 0){
        // Copy to temporary save file
        engine_file_write(1, &character, 1);
    }

    // Close read and write files (delete
    // old file and rename temporary file)
    engine_save_end();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_save_obj, engine_save);


// Doesn't work since some things are passed by value (int) and instead of new version needs to be returned
// STATIC mp_obj_t engine_save_restore(mp_obj_t entry_name_obj, mp_obj_t obj){
//     ENGINE_INFO_PRINTF("EngineSave: Restoring");    

//     engine_restore_start();

//     // Setup for finding line to save new data at
//     const char *entry_name = mp_obj_str_get_str(entry_name_obj);
//     GET_STR_LEN(entry_name_obj, entry_name_len);

//     // Seek until end of file or until the end of entry name
//     uint32_t position = engine_file_seek_until(0, entry_name, entry_name_len);

//     // If we're not at the end of the file, then
//     // we found the name, get the type and edit
//     // the data inside the object with the restored
//     // data
//     if(position != reading_file_size){
//         position = engine_file_seek_until(0, "TYPE=", 5);

//         // Read the 3 digit type string (plus the newline)
//         engine_file_read(0, buffer, 4);

//         // Determine entry type and restore from buffer to object
//         uint8_t entry_type = NONE;
//         if(strncmp(buffer, "STR", 3) == 0){
//             entry_type = STR;
//         }else if(strncmp(buffer, "INT", 3) == 0){
//             entry_type = INT;
//         }else if(strncmp(buffer, "FLT", 3) == 0){
//             entry_type = FLT;
//         }

//         // Read the entry into the buffer
//         char character = ' ';
//         uint8_t restore_index = 0;

//         while(character != '\n'){
//             engine_file_read(0, &character, 1);
//             buffer[restore_index] = character;
//             restore_index++;
//         }

//         // Restore data from read buffer to object
//         if(entry_type == STR){
//             // mp_obj_str_set_data()
//             // mp_obj_str_make_new()
//         }else if(entry_type == INT){
//             if(mp_obj_is_exact_type(obj, &mp_type_int)){
//                 ENGINE_FORCE_PRINTF("TEST0");
//             }else if(mp_obj_is_small_int(obj)){
//                 ENGINE_FORCE_PRINTF("%p", obj);
//                 // ENGINE_FORCE_PRINTF("TEST1");
//                 // *((mp_int_t*)obj) = 1;
//                 // MP_OBJ_SMALL_INT_VALUE()
//                 // mp_int_t i = *((mp_int_t*)obj);
//                 // i = strtol(buffer, NULL, 10);
//                 // ENGINE_FORCE_PRINTF("%d", i);
//                 // *((mp_int_t*)obj) = MP_OBJ_NEW_SMALL_INT(999);
//                 ENGINE_FORCE_PRINTF("TEST4");
//             }else{
//                 ENGINE_FORCE_PRINTF("TEST2");
//             }
//         }else if(entry_type == FLT){
//             ((mp_obj_float_t*)obj)->value = strtof(buffer, NULL);
//         }
//     }

//     engine_restore_end();

//     return mp_const_none;
// }
// MP_DEFINE_CONST_FUN_OBJ_2(engine_save_restore_obj, engine_save_restore);


STATIC mp_obj_t engine_save_load(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("EngineSave: Loading");

    mp_obj_t entry_name_obj = args[0];
    mp_obj_t default_value = mp_const_none;

    // If a default value was passed, set that instead of None
    if(n_args == 2){
        default_value = args[1];
    }

    if(mp_obj_is_str(entry_name_obj) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: entry name is not a string, cannot load!"));
    }

    engine_load_start();

    // Setup for finding line to save new data at
    const char *entry_name = mp_obj_str_get_str(entry_name_obj);
    GET_STR_LEN(entry_name_obj, entry_name_len);

    // Seek until end of file or until the end of entry name
    uint32_t position = engine_file_seek_until(0, entry_name, entry_name_len);

    // If we're not at the end of the file, then
    // we found the name, get the type and edit
    // the data inside the object with the restored
    // data
    if(position != reading_file_size){
        position = engine_file_seek_until(0, "TYPE=", 5);

        // Read the 3 digit type string (plus the newline)
        engine_file_read(0, buffer, 4);

        // Determine entry type
        uint8_t entry_type = NONE;
        if(strncmp(buffer, "STR", 3) == 0){
            entry_type = STR;
        }else if(strncmp(buffer, "INT", 3) == 0){
            entry_type = INT;
        }else if(strncmp(buffer, "FLT", 3) == 0){
            entry_type = FLT;
        }

        // Read the entry into the buffer
        char character = ' ';
        uint16_t restore_index = 0;

        while(character != '\r' && character != '\n'){
            engine_file_read(0, &character, 1);
            buffer[restore_index] = character;
            restore_index++;

            if(restore_index >= SAVE_ENTRY_LENGTH_MAX){
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Trying to load an entry that will overflow an internal buffer! Entry too long!"));
            }
        }

        // Restore data from read buffer to object
        if(entry_type == STR){
            return mp_obj_new_str_of_type(&mp_type_str, buffer, restore_index-1);   // minus 1 to remove newline
        }else if(entry_type == INT){
            return mp_obj_new_int(strtol(buffer, NULL, 10));
        }else if(entry_type == FLT){
            return mp_obj_new_float(strtof(buffer, NULL));
        }
    }

    engine_load_end();

    // If we didn't return anything else then the
    // entry must have not been saved before,
    // return the default value
    return default_value;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_save_load_obj, 1, 2, engine_save_load);


STATIC mp_obj_t engine_save_delete(mp_obj_t entry_name_obj){
    ENGINE_INFO_PRINTF("EngineSave: Deleting entry");

    if(mp_obj_is_str(entry_name_obj) == false){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: entry name is not a string, cannot delete!"));
    }

    // Open read and write files (create read file too if needed)
    engine_save_start();

    // Setup for finding line to delete data at
    const char *entry_name = mp_obj_str_get_str(entry_name_obj);
    GET_STR_LEN(entry_name_obj, entry_name_len);
    uint8_t entry_name_index = 0;
    
    // Temporary for copying
    char character = ' ';

    // Figure out where the entry name is
    // so that it can be deleted as well
    // as the data
    uint32_t entry_name_end_index = engine_file_seek_until(0, entry_name, entry_name_len);
    uint32_t entry_name_start_index = entry_name_end_index - entry_name_len - 5;    /// Minus 5 for `Name=`

    // If the name was found we won't be at the end of the file
    if(entry_name_end_index != reading_file_size){
        // Copy from start to `entry_name_start_index` 1 byte at a time
        engine_file_seek(0, 0, MP_SEEK_SET);
        uint32_t bytes_copied = 0;

        while(engine_file_read(0, &character, 1) != 0 && bytes_copied < entry_name_start_index){
            engine_file_write(1, &character, 1);
            bytes_copied++;
        }
    }

    // In the reading file, skip the old overwritten
    // data and copy the rest of the file to temporary
    uint32_t position = engine_file_seek_until(0, "NAME=", 5);

    // Make sure we aren't really at the end of the
    // file, if not, go back to before `NAME=` and copy
    if(position != reading_file_size){
        engine_file_seek(0, -5, MP_SEEK_CUR);
    }

    // Copy the rest of the file
    while(engine_file_read(0, &character, 1) != 0){
        // Copy to temporary save file
        engine_file_write(1, &character, 1);
    }

    // Close read and write files (delete
    // old file and rename temporary file)
    engine_save_end();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_save_delete_obj, engine_save_delete);


STATIC mp_obj_t engine_save_delete_location(){
    ENGINE_INFO_PRINTF("EngineSave: Deleting save location!");
    if(engine_file_exists(&current_location)){
        engine_file_remove(&current_location);
    } 
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_save_delete_location_obj, engine_save_delete_location);


STATIC mp_obj_t engine_save_module_init(){
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
    // { MP_OBJ_NEW_QSTR(MP_QSTR_restore), (mp_obj_t)&engine_save_restore_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_load), (mp_obj_t)&engine_save_load_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_delete), (mp_obj_t)&engine_save_delete_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_delete_location), (mp_obj_t)&engine_save_delete_location_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_location), (mp_obj_t)&engine_set_location_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT(mp_module_engine_save_globals, engine_save_globals_table);

const mp_obj_module_t engine_save_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_save_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_save, engine_save_user_cmodule);
