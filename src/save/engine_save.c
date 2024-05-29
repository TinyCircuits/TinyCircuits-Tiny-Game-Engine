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
#define TABLE_KEY_COUNT_LEN 2

#define TABLE_SIZE TABLE_THUMBY_CLR_SCREEN_LEN + TABLE_VERSION_LEN + TABLE_KEY_COUNT_LEN

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


void engine_save_start_read_write(){
    // Open the the file to read from and get size
    engine_file_open_read(0, &current_location);
    reading_file_size = engine_file_size(0);

    // Open the file to write to (temporary)
    engine_file_open_create_write(1, &temporary_location);

    // Seek to start of files, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
    engine_file_seek(1, 0, MP_SEEK_SET);
}

void engine_save_stop_read_write(){
    // Close the reading and writing files
    engine_file_close(0);
    engine_file_close(1);

    // Remove the file we read data from and copied
    engine_file_remove(&current_location);

    // Rename temporary file to what we just deleted
    engine_file_rename(&temporary_location, &current_location);
}


void engine_save_start_read(){
    // Open the the file to read from and get size
    engine_file_open_read(0, &current_location);
    reading_file_size = engine_file_size(0);

    // Seek to start of file, just in case
    engine_file_seek(0, 0, MP_SEEK_SET);
}


void engine_save_stop_read(){
    // Close the reading and writing files
    engine_file_close(0);
}


void engine_save_write_table(uint8_t file_index, uint16_t save_version, uint16_t key_count){
    engine_file_write(file_index, THUMBY_CLR_SCREEN, TABLE_THUMBY_CLR_SCREEN_LEN);
    engine_file_write(file_index, &save_version, TABLE_VERSION_LEN);
    engine_file_write(file_index, &key_count, TABLE_KEY_COUNT_LEN);
}


uint32_t engine_save_get_entry_data_len(mp_obj_t entry){
    if(mp_obj_is_str(entry)){
        return ((mp_obj_str_t*)entry)->len;
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


void engine_save_write_bucket_entry(uint8_t file_index, mp_obj_t entry, const char* entry_name, uint16_t entry_name_len){
    uint32_t entry_len = engine_save_get_entry_data_len(entry);

    // Each entry in each bucket is prepended with this metadata:
    //  * entry_name_len
    //  * entry_name
    //  * entry_len (amount of data in the entry)
    engine_file_write(file_index, &entry_name_len, 2);
    engine_file_write(file_index, entry_name, entry_name_len);
    engine_file_write(file_index, &entry_len, 4);

    // Write the entry data
    if(mp_obj_is_str(entry)){
        engine_file_write(file_index, ((mp_obj_str_t*)entry)->data, entry_len);
    }else if(mp_obj_is_int(entry)){
        engine_file_write(file_index, (mp_int_t*)entry, entry_len);
    }else if(mp_obj_is_float(entry)){
        engine_file_write(file_index, &((mp_obj_float_t*)entry)->value, entry_len);
    }else if(mp_obj_is_type(entry, &vector2_class_type)){
        engine_file_write(file_index, &((vector2_class_obj_t*)entry)->x.value, 4);
        engine_file_write(file_index, &((vector2_class_obj_t*)entry)->y.value, 4);
    }else if(mp_obj_is_type(entry, &vector3_class_type)){
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->x.value, 4);
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->y.value, 4);
        engine_file_write(file_index, &((vector3_class_obj_t*)entry)->z.value, 4);
    }else if(mp_obj_is_type(entry, &color_class_type)){
        engine_file_write(file_index, &((color_class_obj_t*)entry)->r.value, 4);
        engine_file_write(file_index, &((color_class_obj_t*)entry)->g.value, 4);
        engine_file_write(file_index, &((color_class_obj_t*)entry)->b.value, 4);
    }else if(mp_obj_is_type(entry, &mp_type_bytearray)){
        engine_file_write(file_index, ((mp_obj_array_t*)entry)->items, entry_len);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: Saving this type of object is not implemented!"));
    }
}


void engine_save_get_table_info(uint8_t file_index, uint16_t *version, uint16_t *key_count){
    // Read entire table from save file into ram
    engine_file_read(file_index, buffer, TABLE_SIZE);

    // Make sure this is still a correct save file
    if(strncmp(buffer, THUMBY_CLR_SCREEN, TABLE_THUMBY_CLR_SCREEN_LEN) != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: file at location is not a save file! Error while saving..."));
    }

    // Fill in info
    memcpy(version,     buffer+TABLE_THUMBY_CLR_SCREEN_LEN,                                     TABLE_VERSION_LEN);
    memcpy(key_count,   buffer+TABLE_THUMBY_CLR_SCREEN_LEN+TABLE_VERSION_LEN,                   TABLE_KEY_COUNT_LEN);
}


void engine_save_set_file_location(const char *location, uint32_t location_len, uint16_t key_count){
    // Cannot be larger than this since `temp-` takes up 5 characters
    if(location_len > SAVE_LOCATION_LENGTH_MAX-5){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path too long (max length: 250)"));
    }

    // Set these equal to each other, for now
    current_location.len = location_len;
    temporary_location.len = location_len;

    // Copy base files name to current and temporary file names
    memcpy(current_location.data, location, current_location.len);
    memcpy(temporary_location.data, location, current_location.len);

    // Append `-temp` to end of temporary filename and increase length to account for it
    memcpy(temporary_location.data + temporary_location.len, "-temp", 5);
    temporary_location.len += 5;

    // If the file we are going to read from does not exist already, create it
    // and write the initial table and special data to it
    if(engine_file_exists(&current_location) == false){
        engine_file_open_create_write(0, &current_location);
        engine_save_write_table(0, SAVE_VERSION, key_count);

        // Write the empty keys
        uint32_t offset_default = 0;
        for(uint16_t key_index=0; key_index<key_count; key_index++){
            engine_file_write(0, &offset_default, 4);
        }

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


void engine_save_del_set_location(){
    if(engine_file_exists(&current_location)){
        engine_file_remove(&current_location);
    } 
}


void engine_save_entry(const char* entry_name, uint16_t entry_name_len, mp_obj_t entry){
    // STEP #1: Open files and get table info and copy to new file
    engine_save_start_read_write();

    uint16_t version = 0;
    uint16_t key_count = 0;

    engine_save_get_table_info(0, &version, &key_count);
    engine_save_write_table(1, version, key_count);
    ENGINE_FORCE_PRINTF("version=%d key_count=%d", version, key_count);

    // STEP #2: Use micropython hashing
    // function to get a pretty good hash
    uint32_t key = qstr_compute_hash(entry_name, entry_name_len);
    
    // Only have so many fixed slots for offsets
    // in file. If key_count=256 then there are
    // indices at 0 to 255, get the index into
    // the offsets
    key = key % key_count;

    // STEP #3: Now figure out how many more bytes to
    // seek before we reach the bytes for the offset
    // into the bucket section
    // (each hash/key/offset is 32-bits or 4 bytes and
    // we're already at the end of the table info section)
    uint32_t rel_file_offset_into_offsets = key * 4;
    engine_file_seek(0, rel_file_offset_into_offsets, MP_SEEK_CUR);

    // STEP #4: Read the offset for the offset into the buckets
    uint32_t abs_file_offset_into_buckets = 0;
    engine_file_read(0, &abs_file_offset_into_buckets, 4);
    ENGINE_FORCE_PRINTF("abs_file_offset_into_buckets=%ld", abs_file_offset_into_buckets);

    // STEP #5: Seek to the start of the keys again (for copying/searching)
    engine_file_seek(0, TABLE_SIZE, MP_SEEK_SET);

    // STEP #6: If the absolute offset is 0, then this offset has not
    // been set before, come up with a new offset for these keys to
    // lead to. Otherwise, go to offset and start append process
    if(abs_file_offset_into_buckets == 0){
        abs_file_offset_into_buckets = reading_file_size;   // Will start new bucket at end of file

        // Unless we find the key we want to set, copy keys
        uint32_t offset = 0;
        for(uint32_t key_index=0; key_index<key_count; key_index++){
            if(key_index == key){
                // Set
                engine_file_write(1, &abs_file_offset_into_buckets, 4);
            }else{
                // Copy
                engine_file_read(0, &offset, 4);
                engine_file_write(1, &offset, 4);
            }
        }
    }else{
        // Only copy keys
        engine_file_copy_amount_from_to(0, 1, key_count*4, buffer, BUFFER_LENGTH_MAX);
    }

    // STEP #7: At this point both the read and write files
    // are at the end of the keys/offset table, copy all data
    // from the end of keys/offsets (start of data) to start
    // of the bucket we want to put an entry into
    engine_file_copy_from_to_until(0, 1, abs_file_offset_into_buckets, buffer, BUFFER_LENGTH_MAX);

    // STEP #8: At the start of the bucket now where
    // each entry in the bucket has the format:
    // bucket_len,entry_name_len,entry_name,entry_data_len,entry,entry_name_len,entry_name,entry_data_len,entry
    // Get the bucket size
    uint32_t bucket_len = 0;                // This includes everything `entry_name_len,entry_name,entry_len,entry` ...
    engine_file_read(0, &bucket_len, 4);
    uint32_t bucket_amount_read = 0;
    bool entry_found = false;
    uint16_t read_entry_name_len = 0;
    uint32_t entry_data_len = 0;

    ENGINE_FORCE_PRINTF("bucket_len=%ld", bucket_len);

    // Go through buckets and find the entry
    while(bucket_amount_read < bucket_len && entry_found == false){
        // Read the length of the entry name
        bucket_amount_read += engine_file_read(0, &read_entry_name_len, 2);
        
        // Setup to read the entry name
        uint16_t entry_name_index = 0;
        char entry_name_char = ' ';

        // Read the entry name until missing a char in the
        // sequence, read until end of entry, or the entry
        // name is found
        while(entry_name_index < read_entry_name_len){
            // Read a entry_name character
            bucket_amount_read += engine_file_read(0, &entry_name_char, 1);

            if(entry_name_char != entry_name[entry_name_index]){
                // Did not find the entry, skip the rest of the entry name
                engine_file_seek(0, read_entry_name_len-entry_name_index, MP_SEEK_CUR);
                
                // Read the entry_data_Len
                engine_file_read(0, &entry_data_len, 4);

                // Seek past the entry data
                engine_file_seek(0, entry_data_len, MP_SEEK_CUR);

                // Add up where we are in the bucket
                bucket_amount_read += read_entry_name_len + 4 + entry_data_len;
                break;
            }else{
                entry_name_index++;
            }

            if(entry_name_index == read_entry_name_len){
                // Mark as found and stop saving routine
                entry_found = true;
                ENGINE_FORCE_PRINTF("FOUND!");
                break;
            }
        }
    }

    // STEP #9: Entry in bucket may or may not have been found,
    // update bucket size depending on which occurred
    if(entry_found){
        // Add the difference in entry size to the bucket length.
        // If the old `entry_data_len` is now smaller than the new
        // entry size, the bucket gets larger in size
        bucket_len += engine_save_get_entry_data_len(entry) - entry_data_len;
    }else{
        bucket_len += engine_save_get_entry_data_len(entry);
    }

    // STEP #10: Write the updated bucket size
    engine_file_write(1, &bucket_len, 4);

    // STEP #11: Copy data from before and upto the entry (inside bucket)
    engine_file_seek(0, abs_file_offset_into_buckets, MP_SEEK_SET);
    engine_file_copy_amount_from_to(0, 1, bucket_amount_read-(read_entry_name_len-2), buffer, BUFFER_LENGTH_MAX);

    // STEP #12: Save entry
    engine_save_write_bucket_entry(1, entry, entry_name, entry_name_len);

    // STEP #13: Copy the rest of the data from the old file to the new file
    engine_file_copy_from_to_until(0, 1, reading_file_size, buffer, BUFFER_LENGTH_MAX);

    engine_save_stop_read_write();
}


void engine_load_entry(){

}


void engine_load_entry_into(){

}


void engine_delete_entry(){

}