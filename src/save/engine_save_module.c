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

enum entry_types {NONE=0, STR=1, INT=2, FLT=3, VC2=4, VC3=5, CLR=6, BAR=7};


/* --- doc ---
   NAME: set_location
   ID: engine_save_set_location
   DESC: Sets the current file location that entries will be saved to (required that this is don't before trying to save, load, or delete entries)
   PARAM:   [type=str]  [name=filepath]   [value=str]
   RETURN: None
*/
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


// TODO: check lengths of data when saving! Except for bytearray and str, those can be any length
void engine_save_store_raw(uint8_t file_index, mp_obj_t obj){
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
    }else if(mp_obj_is_type(obj, &vector2_class_type)){
        engine_file_write(file_index, "TYPE=VC2\n", 9);

        uint32_t len = 0;
        
        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f ", ((mp_obj_float_t)((vector2_class_obj_t*)obj)->x).value);
        engine_file_write(file_index, buffer, len);

        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f", ((mp_obj_float_t)((vector2_class_obj_t*)obj)->y).value);
        engine_file_write(file_index, buffer, len);

    }else if(mp_obj_is_type(obj, &vector3_class_type)){
        engine_file_write(file_index, "TYPE=VC3\n", 9);

        uint32_t len = 0;
        
        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f ", ((mp_obj_float_t)((vector3_class_obj_t*)obj)->x).value);
        engine_file_write(file_index, buffer, len);

        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f ", ((mp_obj_float_t)((vector3_class_obj_t*)obj)->y).value);
        engine_file_write(file_index, buffer, len);

        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f", ((mp_obj_float_t)((vector3_class_obj_t*)obj)->z).value);
        engine_file_write(file_index, buffer, len);
    }else if(mp_obj_is_type(obj, &color_class_type)){
        engine_file_write(file_index, "TYPE=CLR\n", 9);

        uint32_t len = 0;
        
        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f ", ((mp_obj_float_t)((color_class_obj_t*)obj)->r).value);
        engine_file_write(file_index, buffer, len);

        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f ", ((mp_obj_float_t)((color_class_obj_t*)obj)->g).value);
        engine_file_write(file_index, buffer, len);

        len = snprintf(buffer, SAVE_ENTRY_LENGTH_MAX, "%.15f", ((mp_obj_float_t)((color_class_obj_t*)obj)->b).value);
        engine_file_write(file_index, buffer, len);
    }else if(mp_obj_is_type(obj, &mp_type_bytearray)){
        engine_file_write(file_index, "TYPE=BAR\n", 9);
        mp_obj_array_t *array = obj;
        engine_file_write(file_index, array->items, array->len);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Saving this type of object is not implemented!"));
    }

    // After the type, write the newline
    engine_file_write(1, "\n", 1);
}


mp_obj_t engine_save_load_raw(uint8_t file_index, uint8_t entry_type, uint32_t available_data_len){
    // Restore data from read buffer to object
    if(entry_type == STR){
        // Do all of this so that strings can be any length
        // See `mp_obj_new_str_copy` in py/objstr.c
        mp_obj_str_t *str = mp_obj_malloc(mp_obj_str_t, &mp_type_str);
        str->len = available_data_len;

        // len will include the newline from the save file at first
        // but then will be replaced by `\0` later
        byte *data = m_new(byte, available_data_len);

        // Seek to start of data and read into new str memory (copy)
        engine_file_seek(0, -available_data_len, MP_SEEK_CUR);
        engine_file_read(0, data, available_data_len);
        data[available_data_len-1] = '\0';   // Replace \n with \0

        // Search for interned/cached string without \0
        // // See `mp_obj_new_str` in py/objstr.c
        qstr q = qstr_find_strn(data, available_data_len-1);

        if(q != MP_QSTRnull){
            // qstr with this data already exists, return reference
            // unique str after deleting the string we just allocated
            m_del(byte, data, available_data_len);
            return MP_OBJ_NEW_QSTR(q);
        }else{
            // no existing qstr, don't make one
            // and return the new string object
            str->hash = qstr_compute_hash(data, available_data_len-1);   // has without \0
            str->data = data;
            return str;
        }
    }else if(entry_type == INT){
        return mp_obj_new_int(strtol(buffer, NULL, 10));
    }else if(entry_type == FLT){
        return mp_obj_new_float(strtof(buffer, NULL));
    }else if(entry_type == VC2){
        char *start;
        mp_obj_t x = mp_obj_new_float(strtof(buffer, &start));
        mp_obj_t y = mp_obj_new_float(strtof(start, NULL));
        return vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){x, y});
    }else if(entry_type == VC3){
        char *start;
        mp_obj_t x = mp_obj_new_float(strtof(buffer, &start));
        mp_obj_t y = mp_obj_new_float(strtof(start, &start));
        mp_obj_t z = mp_obj_new_float(strtof(start, NULL));
        return vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){x, y, z});
    }else if(entry_type == CLR){
        char *start;
        mp_obj_t r = mp_obj_new_float(strtof(buffer, &start));
        mp_obj_t g = mp_obj_new_float(strtof(start, &start));
        mp_obj_t b = mp_obj_new_float(strtof(start, NULL));
        return color_class_new(&color_class_type, 3, 0, (mp_obj_t[]){r, g, b});
    }else if(entry_type == BAR){
        // Create empty bytearray first (see `bytearray_make_new` and `array_new` in py/objarray.c)
        mp_obj_array_t *array = m_new_obj(mp_obj_array_t);
        array->base.type = &mp_type_bytearray;
        array->typecode = BYTEARRAY_TYPECODE;
        array->free = 0;
        array->len = available_data_len-1;   // Minus 1 to discard newline
        array->items = m_new(byte, array->len);
        memset(array->items, 0, array->len);

        // Seek to start of data and read into new bytearray memory (copy)
        engine_file_seek(0, -available_data_len, MP_SEEK_CUR);
        engine_file_read(0, array->items, array->len);
        return (mp_obj_t)array;
    }
}


mp_obj_t engine_save_load_into_raw(uint8_t file_index, mp_obj_t obj, uint8_t entry_type, uint32_t available_data_len, uint32_t max_len){
    // Restore data from read buffer to object
    if(entry_type == STR && mp_obj_is_str(obj)){
        const char* obj_str = mp_obj_str_get_str(obj);
        GET_STR_LEN(obj, obj_str_len);

        uint32_t size = min3(obj_str_len, available_data_len, max_len);

        // Seek to start of data and read into new str memory (copy)
        engine_file_seek(0, -available_data_len, MP_SEEK_CUR);        
        engine_file_read(0, obj_str, size);
        return mp_obj_new_int(size);
    }else if(entry_type == FLT && mp_obj_is_float(obj)){
        ((mp_obj_float_t*)obj)->value = strtof(buffer, NULL);
    }else if(entry_type == VC2 && mp_obj_is_type(obj, &vector2_class_type)){
        char *start;
        vector2_class_obj_t *vec2 = obj;
        vec2->x.value = strtof(buffer, &start);
        vec2->y.value = strtof(start, NULL);
    }else if(entry_type == VC3 && mp_obj_is_type(obj, &vector3_class_type)){
        char *start;
        vector3_class_obj_t *vec3 = obj;
        vec3->x.value = strtof(buffer, &start);
        vec3->y.value = strtof(start, &start);
        vec3->z.value = strtof(start, NULL);
    }else if(entry_type == CLR && mp_obj_is_type(obj, &color_class_type)){
        char *start;
        color_class_obj_t *color = obj;
        color->r.value = strtof(buffer, &start);
        color->g.value = strtof(start, &start);
        color->b.value = strtof(start, NULL);
    }else if(entry_type == BAR  && mp_obj_is_type(obj, &mp_type_bytearray)){
        mp_obj_array_t *array = obj;
        engine_file_seek(0, -available_data_len, MP_SEEK_CUR); 

        uint32_t size = min3(array->len, available_data_len-1, max_len);    // minus 1 to discard newline

        // Seek to start of data and read into new str memory (copy)       
        engine_file_read(0, array->items, size);

        return mp_obj_new_int(size);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Tried to load data into an object with an unsupported type or the types do not match!"));
    }

    return mp_const_none;
}


mp_obj_t engine_save_load_or_into(size_t n_args, const mp_obj_t *args, bool load_into){
    ENGINE_INFO_PRINTF("EngineSave: Loading");

    mp_obj_t entry_name_obj = args[0];
    mp_obj_t value = mp_const_none; // Acts as `default` or as `value` for data to be loaded into

    // If a default value was passed, set that instead of None
    if(n_args >= 2){
        value = args[1];
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
        }else if(strncmp(buffer, "VC2", 3) == 0){
            entry_type = VC2;
        }else if(strncmp(buffer, "VC3", 3) == 0){
            entry_type = VC3;
        }else if(strncmp(buffer, "CLR", 3) == 0){
            entry_type = CLR;
        }else if(strncmp(buffer, "BAR", 3) == 0){
            entry_type = BAR;
        }

        // Read the entry into the buffer
        char character = ' ';
        uint32_t restore_index = 0;

        while(character != '\r' && character != '\n'){
            engine_file_read(0, &character, 1);

            // If `BAR` or 'STR' then we only want to count amount of data for now
            // since the buffer is limited and small for other types that turn into
            // something else
            if(entry_type != BAR && entry_type != STR){
                if(restore_index >= SAVE_ENTRY_LENGTH_MAX){
                    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Trying to load an entry that will overflow an internal buffer! Entry too long!"));
                }

                buffer[restore_index] = character;
            }

            restore_index++;
        }

        if(load_into == false){
            return engine_save_load_raw(0, entry_type, restore_index);
        }else{
            uint32_t max_len = UINT32_MAX;

            if(n_args >= 3){
                max_len = mp_obj_get_int(args[2]);
            }

            return engine_save_load_into_raw(0, value, entry_type, restore_index, max_len);
        }
    }

    engine_load_end();

    // If we didn't return anything else then the
    // entry must have not been saved before,
    // return the default value
    return value;
}


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
    engine_save_store_raw(1, obj);

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


/* --- doc ---
   NAME: load
   ID: engine_save_load
   DESC: Loads and returns the value/object from the save file from the given entry name. This allocates new memory which can be large depending on the data (str and bytearray). If no `default` is passed and the entry is not found, None is returned
   PARAM:   [type=str]  [name=entry_name]   [value=str]
   PARAM:   [type=any]  [name=default]      [value=any (optional, if the `entry_name` isn't found then this default value will be returned instead)]
   RETURN: str, bytearray, int, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}, or None
*/
STATIC mp_obj_t engine_save_load(size_t n_args, const mp_obj_t *args){
    return engine_save_load_or_into(n_args, args, false);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_save_load_obj, 1, 2, engine_save_load);


/* --- doc ---
   NAME: load_into
   ID: engine_save_load_into
   DESC: Similar to `engine_save.load()` but instead of returning a copy of the object it loads data from the save file to an existing object. Less types are supported when using this method but it should save memory. If the optional `max_len` value is passed, then only that max amount of data will be copied from the save file to the `value`. If `max_len` is not passed, as much data that can fit into value will be copied. If the optional `start` value is passed, then `max_len` number of bytes from that index will be copied to `value` (`start` is only supported for `str` and `bytearray` types).
   PARAM:   [type=str]                                                                                  [name=entry_name]   [value=str]
   PARAM:   [type=str, bytearray, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}]      [name=value]        [value=str, bytearray, float, {ref_link:Vector2}, {ref_link:Vector3}, {ref_link:Color}]
   PARAM:   [type=int]                                                                                  [name=max_len]      [value=0 to any positive number (optional)]
   RETURN: int (number of bytes copied from the save)
*/
STATIC mp_obj_t engine_save_load_into(size_t n_args, const mp_obj_t *args){
    return engine_save_load_or_into(n_args, args, true);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_save_load_into_obj, 1, 4, engine_save_load_into);


/* --- doc ---
   NAME: delete
   ID: engine_save_delete
   DESC: Deletes the save entry inside the save file for the given entry name
   PARAM:   [type=str]  [name=entry_name]   [value=str]
   RETURN: None
*/
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


/* --- doc ---
   NAME: delete_location
   ID: engine_save_delete_location
   DESC: Deletes the set save file
   RETURN: None
*/
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
   ATTR:   [type=function]    [name={ref_link:engine_save_set_location}]        [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_delete_location}]     [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_save}]                [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_load}]                [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_load_into}]           [value=function]
   ATTR:   [type=function]    [name={ref_link:engine_save_delete}]              [value=function]
*/
STATIC const mp_rom_map_elem_t engine_save_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_save) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_save_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_save), (mp_obj_t)&engine_save_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_load), (mp_obj_t)&engine_save_load_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_load_into), (mp_obj_t)&engine_save_load_into_obj },
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
