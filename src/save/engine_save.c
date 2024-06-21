#include "engine_save.h"

#include "utility/engine_mp.h"
#include "math/engine_math.h"
#include "utility/engine_file.h"
#include "debug/debug_print.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_color.h"
#include "versions.h"

#include "py/objstr.h"
#include "py/objint.h"
#include "py/objarray.h"
#include "py/binary.h"


/*                      #### ENGINE SAVE FORMAT ####

            indices (size)    |      data       |                                                 desc.
      0  ~ 15 (16 bytes)      |THUMBY_CLR_SAVE\n|    Unique string: 16 bytes indicating this is a Thumby Color save file (newline for humans to read)
      16 ~ 17 (2  bytes)      |UINT_16          |          Version: 2 bytes indicating the version of this save file (can be changed in the firmware if needed)
      18 ~ 19 (2  bytes)      |UINT_16          | Key/offset count: 2 bytes key count for when string keys are hashed and reduced to an index into the subsequent offset table. Higher key count means more flash taken up (256*4=1000) but less searching during collisions and vice versa
      20 ~ offset_end         |UINT_32 array    |   Bucket offsets: Depending on previous 2 bytes, if key_count=128 and offset_size=4 bytes (always true) then 128*4=512 bytes of offset bytes
      offset_end ~ bucket_end |Variable         |          Buckets: Bucket offsets are seek positions to starts of buckets in this part of the data. Each bucket consists of type,offset,data_length,key_length,key pairs
*/

#define SAVE_LOCATION_LENGTH_MAX 256
#define BUFFER_LENGTH_MAX 256

#define TABLE_THUMBY_CLR_SCREEN_LEN 4
#define TABLE_VERSION_LEN 2

#define TABLE_SIZE TABLE_THUMBY_CLR_SCREEN_LEN + TABLE_VERSION_LEN

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
char buffer[BUFFER_LENGTH_MAX];

const char THUMBY_CLR_SCREEN[TABLE_THUMBY_CLR_SCREEN_LEN] = {'T', 'H', 'S', 'V'};

uint32_t reading_file_size = 0;

enum entry_types {SAVE_NONE=0, SAVE_STRING=1, SAVE_INTEGER=2, SAVE_FLOAT=3, SAVE_VECTOR2=4, SAVE_VECTOR3=5, SAVE_COLOR=6, SAVE_BYTEARRAY=7};


void engine_saving_start_read_write(){
    // Open the the file to read from and get size
    engine_file_open_read(0, &current_location);
    reading_file_size = engine_file_size(0);

    // Open the file to write to (temporary)
    engine_file_open_create_write(1, &temporary_location);

    // Seek to start of files, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
    engine_file_seek(1, 0, MP_SEEK_SET);
}

void engine_saving_stop_read_write(){
    // Close the reading and writing files
    engine_file_close(0);
    engine_file_close(1);

    // Remove the file we read data from and copied
    engine_file_remove(&current_location);

    // Rename temporary file to what we just deleted
    engine_file_rename(&temporary_location, &current_location);
}


void engine_saving_start_read(){
    // Open the the file to read from and get size
    engine_file_open_read(0, &current_location);
    reading_file_size = engine_file_size(0);

    // Seek to start of file, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
}


void engine_saving_stop_read(){
    // Close the reading and writing files
    engine_file_close(0);
}


uint32_t engine_saving_get_entry_data_len(mp_obj_t entry){
    if(mp_obj_is_str(entry)){
        GET_STR_LEN(entry, str_len);
        return str_len;
    }else if(mp_obj_is_int(entry)){
        return 4;
    }else if(mp_obj_is_float(entry)){
        return 4;
    }else if(mp_obj_is_type(entry, &vector2_class_type)){
        return 4 + 4;
    }else if(mp_obj_is_type(entry, &vector3_class_type)){
        return 4 + 4 + 4;
    }else if(mp_obj_is_type(entry, &color_class_type)){
        return 4 + 4 + 4;
    }else if(mp_obj_is_type(entry, &mp_type_bytearray)){
        return ((mp_obj_array_t*)entry)->len;
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Saving this type of object is not implemented!"));
    }
}


mp_obj_t engine_saving_read_entry(uint8_t file_index, uint32_t entry_data_len, uint8_t entry_data_type){
    switch(entry_data_type){
        case SAVE_STRING:
        {
            // Do all of this so that strings can be any length
            // See `mp_obj_new_str_copy` in py/objstr.c
            mp_obj_str_t *str = mp_obj_malloc(mp_obj_str_t, &mp_type_str);
            str->len = entry_data_len;

            // Create the string data
            byte *data = m_new(byte, entry_data_len);

            // Read into new str memory (copy)
            engine_file_read(file_index, data, entry_data_len);

            // Search for interned/cached string without
            // See `mp_obj_new_str` in py/objstr.c
            qstr q = qstr_find_strn((char*)data, entry_data_len);

            if(q != MP_QSTRnull){
                // qstr with this data already exists, return reference
                // unique str after deleting the string we just allocated
                m_del(byte, (void*)data, entry_data_len);
                return MP_OBJ_NEW_QSTR(q);
            }else{
                // no existing qstr, don't make one
                // and return the new string object
                str->hash = qstr_compute_hash(data, entry_data_len);
                str->data = data;
                return str;
            }
        }
        break;
        case SAVE_INTEGER:
        {
            mp_int_t integer = 0;
            engine_file_read(file_index, &integer, entry_data_len);
            return mp_obj_new_int(integer);
        }
        break;
        case SAVE_FLOAT:
        {
            float flt = 0.0f;
            engine_file_read(file_index, &flt, entry_data_len);
            return mp_obj_new_float(flt);
        }
        break;
        case SAVE_VECTOR2:
        {
            float xf = 0.0f;
            float yf = 0.0f;
            engine_file_read(file_index, &xf, 4);
            engine_file_read(file_index, &yf, 4);
            return vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(xf), mp_obj_new_float(yf)});
        }
        break;
        case SAVE_VECTOR3:
        {
            float xf = 0.0f;
            float yf = 0.0f;
            float zf = 0.0f;
            engine_file_read(file_index, &xf, 4);
            engine_file_read(file_index, &yf, 4);
            engine_file_read(file_index, &zf, 4);
            return vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(xf), mp_obj_new_float(yf), mp_obj_new_float(zf)});
        }
        break;
        case SAVE_COLOR:
        {
            uint16_t value = 0;
            engine_file_read(file_index, &value, 2);
            return color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(value)});
        }
        break;
        case SAVE_BYTEARRAY:
        {
            // Create empty bytearray first (see `bytearray_make_new` and `array_new` in py/objarray.c)
            mp_obj_array_t *array = m_new_obj(mp_obj_array_t);
            array->base.type = &mp_type_bytearray;
            array->typecode = BYTEARRAY_TYPECODE;
            array->free = 0;
            array->len = entry_data_len;
            array->items = m_new(byte, array->len);
            memset(array->items, 0, array->len);
            engine_file_read(0, array->items, array->len);
            return (mp_obj_t)array;
        }
        break;
        default:
        {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Could not load entry! Type unknown!"));
        }
    }

    return mp_const_none;
}


void engine_saving_write_entry(uint8_t file_index, mp_obj_t entry, const byte* entry_name, size_t entry_name_len){
    uint32_t entry_data_len = engine_saving_get_entry_data_len(entry);
    uint8_t entry_data_type = SAVE_NONE;

    // Each entry is prepended with this metadata:
    //  * entry_name_len
    //  * entry_name
    //  * entry_data_len
    engine_file_write(file_index, &entry_name_len, 2);
    engine_file_write(file_index, entry_name, entry_name_len);
    engine_file_write(file_index, &entry_data_len, 4);

    // Write the entry data
    if(mp_obj_is_str(entry)){
        entry_data_type = SAVE_STRING;
        GET_STR_DATA_LEN(entry, str_data, str_len);
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, str_data, entry_data_len);
    }else if(mp_obj_is_int(entry)){
        entry_data_type = SAVE_INTEGER;
        int value = mp_obj_get_int(entry);
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, &value, entry_data_len);
    }else if(mp_obj_is_float(entry)){
        entry_data_type = SAVE_FLOAT;
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, &((mp_obj_float_t*)entry)->value, entry_data_len);
    }else if(mp_obj_is_type(entry, &vector2_class_type)){
        entry_data_type = SAVE_VECTOR2;
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, &((vector2_class_obj_t*)entry)->x.value, 4);
        engine_file_write(file_index, &((vector2_class_obj_t*)entry)->y.value, 4);
    }else if(mp_obj_is_type(entry, &vector3_class_type)){
        entry_data_type = SAVE_VECTOR3;
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->x.value, 4);
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->y.value, 4);
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->z.value, 4);
    }else if(mp_obj_is_type(entry, &color_class_type)){
        entry_data_type = SAVE_COLOR;
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, &((color_class_obj_t*)entry)->value, 2);
    }else if(mp_obj_is_type(entry, &mp_type_bytearray)){
        entry_data_type = SAVE_BYTEARRAY;
        engine_file_write(file_index, &entry_data_type, 1);
        engine_file_write(file_index, ((mp_obj_array_t*)entry)->items, entry_data_len);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Saving this type of object is not implemented!"));
    }
}


void engine_saving_get_meta_table(uint8_t file_index, uint16_t *version){
    // Read entire table from save file into ram
    engine_file_read(file_index, buffer, TABLE_SIZE);

    // Make sure this is still a correct save file
    if(strncmp(buffer, THUMBY_CLR_SCREEN, TABLE_THUMBY_CLR_SCREEN_LEN) != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: file at location is not a save file! Error while saving..."));
    }

    // Fill in info
    memcpy(version, buffer+TABLE_THUMBY_CLR_SCREEN_LEN, TABLE_VERSION_LEN);
}


void engine_saving_save_meta_table(uint8_t file_index, uint16_t save_version){
    engine_file_write(file_index, THUMBY_CLR_SCREEN, TABLE_THUMBY_CLR_SCREEN_LEN);
    engine_file_write(file_index, &save_version, TABLE_VERSION_LEN);
}


uint16_t engine_saving_seek_compare_string_in_file(uint8_t file_index, const byte *str, size_t str_len, bool *entry_name_found){
    // Go through the name from the file and the passed
    // name char by char. If at any point the name does
    // not match, break out of the parsing loop below
    uint16_t str_index = 0;
    char entry_name_char = ' ';

    while(engine_file_read(0, &entry_name_char, 1) != 0 && str_index < str_len){
        if(entry_name_char == str[str_index]){
            str_index++;
        }else{
            *entry_name_found = false;
            break;
        }

        if(str_index == str_len){
            *entry_name_found = true;
            break;
        }
    }

    return str_index;
}


bool engine_saving_seek_copy_to_entry_in_file(uint8_t from_file_index, uint8_t to_file_index, const byte *entry_name, size_t entry_name_len, uint32_t *out_data_len, uint8_t *out_data_type, bool copy){
    bool entry_name_found = false;

    while(entry_name_found == false){
        // Parsing entry names starts by getting name length
        uint16_t entry_name_len = 0;
        engine_file_read(from_file_index, &entry_name_len, 2);

        // If len still zero after reading, must be at end of file,
        // stop reading
        if(entry_name_len == 0){
            break;
        }

        // Seek and find string from where we are currently in
        // the reading file
        uint16_t entry_name_index = engine_saving_seek_compare_string_in_file(from_file_index, entry_name, entry_name_len, &entry_name_found);

        // If the name was found or not, skip to end of entry_name
        // and get the entry_data_len so we can skip or copy it
        uint32_t entry_data_len = 0;
        if(entry_name_found == false) engine_file_seek(from_file_index, (entry_name_len-1)-entry_name_index, MP_SEEK_CUR);
        engine_file_read(from_file_index, &entry_data_len, 4);

        // Read the data type but don't do anything with it
        uint8_t entry_data_type = SAVE_NONE;
        engine_file_read(from_file_index, &entry_data_type, 1);

        // If we found the entry name, skip past old data
        // If did not find name, skip data and keep looking
        // for the entry
        if(entry_name_found){
            // Seek past old data in reading file so it doesn't get
            // copied later
            if(copy) engine_file_seek(from_file_index, entry_data_len, MP_SEEK_CUR);
            if(out_data_len != NULL) *out_data_len = entry_data_len;
            if(out_data_type != NULL) *out_data_type = entry_data_type;

            // Found and replaced, break out
            break;
        }else{
            // entry_name_len + entry_name + entry_data_len + entry_data_type -> data
            uint32_t entry_entire_tag_length = 2+entry_name_len+4+1;

            if(copy){
                engine_file_seek(from_file_index, -(entry_entire_tag_length), MP_SEEK_CUR);
                engine_file_copy_amount_from_to(from_file_index, to_file_index, entry_entire_tag_length+entry_data_len, buffer, BUFFER_LENGTH_MAX);
            }else{
                engine_file_seek(from_file_index, entry_data_len, MP_SEEK_CUR);
            }
        }
    }

    return entry_name_found;
}


void engine_saving_set_file_location(const byte *location, size_t location_len){
    // Cannot be larger than this since `temp-` takes up 5 characters
    if(location_len > SAVE_LOCATION_LENGTH_MAX-5){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path too long (max length: 250)"));
    }

    // Set these equal to each other, for now
    current_location.len = location_len;
    temporary_location.len = location_len;

    // Copy base files name to current and temporary file names
    memcpy((byte*)current_location.data, location, current_location.len);
    memcpy((byte*)temporary_location.data, location, current_location.len);

    // Append `-temp` to end of temporary filename and increase length to account for it
    memcpy((byte*)temporary_location.data + temporary_location.len, "-temp", 5);
    temporary_location.len += 5;

    // If the file we are going to read from does not exist already,
    // create it and write the initial table and special data to it
    if(engine_file_exists(&current_location) == false){
        engine_file_open_create_write(0, &current_location);
        engine_saving_save_meta_table(0, SAVE_VERSION);
        engine_file_close(0);
    }else{
        // Looks likes the file already exists, check that it has the
        // unique sting at the start or error if it does not
        engine_file_open_read(0, &current_location);
        uint8_t read_len = engine_file_read(0, buffer, TABLE_THUMBY_CLR_SCREEN_LEN);

        if(read_len != TABLE_THUMBY_CLR_SCREEN_LEN || strncmp(buffer, THUMBY_CLR_SCREEN, TABLE_THUMBY_CLR_SCREEN_LEN) != 0){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: file at location is not a save file!"));
        }

        engine_file_close(0);
    }
}


void engine_saving_del_set_location(){
    if(engine_file_exists(&current_location)){
        engine_file_remove(&current_location);
    }
}


void engine_saving_save_entry(const byte* entry_name, size_t entry_name_len, mp_obj_t entry){
    // STEP #1: Open files and get table info and copy to new file
    // NOTE: read and write file cursors are at: 0 0
    engine_saving_start_read_write();

    uint16_t version = 0;
    engine_saving_get_meta_table(0, &version);
    engine_saving_save_meta_table(1, version);

    // STEP #2: Go through all keys in file until we find this
    // entry's key str/name
    engine_saving_seek_copy_to_entry_in_file(0, 1, entry_name, entry_name_len, NULL, NULL, true);

    // STEP #3: Save the entry
    engine_saving_write_entry(1, entry, entry_name, entry_name_len);

    // STEP #4: Copy the rest of the file
    engine_file_copy_from_to_until(0, 1, reading_file_size, buffer, BUFFER_LENGTH_MAX);

    engine_saving_stop_read_write();
}


mp_obj_t engine_saving_load_entry(const byte* entry_name, size_t entry_name_len){
    mp_obj_t entry = mp_const_none;

    // STEP #1: Open file to read from and get file version
    // (also checks that the file is a save file)
    engine_saving_start_read();

    uint16_t version = 0;
    engine_saving_get_meta_table(0, &version);

    uint32_t entry_data_len = 0;
    uint8_t entry_data_type = SAVE_NONE;

    // STEP #2: Search for entry and restore
    if(engine_saving_seek_copy_to_entry_in_file(0, 1, entry_name, entry_name_len, &entry_data_len, &entry_data_type, false)){
        entry = engine_saving_read_entry(0, entry_data_len, entry_data_type);
    }

    engine_saving_stop_read();

    return entry;
}


void engine_saving_delete_entry(const byte *entry_name, size_t entry_name_len){
    // STEP #1: Open files and get table info and copy to new file
    // NOTE: read and write file cursors are at: 0 0
    engine_saving_start_read_write();

    uint16_t version = 0;
    engine_saving_get_meta_table(0, &version);
    engine_saving_save_meta_table(1, version);

    // STEP #2: Go through all keys in file until we find this
    // entry's key str/name
    engine_saving_seek_copy_to_entry_in_file(0, 1, entry_name, entry_name_len, NULL, NULL, true);

    // STEP #3: Copy the rest of the file
    engine_file_copy_from_to_until(0, 1, reading_file_size, buffer, BUFFER_LENGTH_MAX);

    engine_saving_stop_read_write();
}