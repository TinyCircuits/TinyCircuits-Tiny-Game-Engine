#include "engine_save.h"

#include "utility/engine_mp.h"
#include "utility/engine_file.h"
#include "versions.h"

/*                      #### ENGINE SAVE FORMAT ####

            indices (size)    |      data       |                                                 desc.
      0  ~ 15 (16 bytes)      |THUMBY_CLR_SAVE\n|    Unique string: 16 bytes indicating this is a Thumby Color save file (newline for humans to read)
      16 ~ 17 (2  bytes)      |UINT_16          |          Version: 2 bytes indicating the version of this save file (can be changed in the firmware if needed)
      18 ~ 21 (4  bytes)      |UINT_32          |      Data length: 4 bytes indicating how large the data section of this file is (updated when entries are added, changed, or deleted)
      22 ~ 23 (2  bytes)      |UINT_16          | Key/offset count: 2 bytes key count for when string keys are hashed and reduced to an index into the subsequent offset table. Higher key count means more flash taken up (256*4=1000) but less searching during collisions and vice versa
      24 ~ offset_end         |UINT_32 array    |   Bucket offsets: Depending on previous 2 bytes, if key_count=128 and offset_size=4 bytes (always true) then 128*4=512 bytes of offset bytes
      offset_end ~ bucket_end |Variable         |          Buckets: Bucket offsets are seek positions to starts of buckets in this part of the data. Each bucket consists of type,offset,data_length,key_length,key pairs
*/

#define SAVE_LOCATION_LENGTH_MAX 256
#define BUFFER_LENGTH_MAX 256

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

const char THUMBY_CLR_SCREEN[16] = {'T', 'H', 'U', 'M', 'B', 'Y', '_', 'C', 'L', 'R', '_', 'S', 'A', 'V', 'E', '\n'};

uint32_t reading_file_size = 0;

enum entry_types {NONE=0, STRING=1, INTEGER=2, FLOAT=3, VECTOR2=4, VECTOR3=5, COLOR=6, BYTEARRAY=7};


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


void engine_save_init_table(uint8_t file_index, uint16_t key_count){
    uint32_t save_version = SAVE_VERSION;
    uint32_t data_length = 0;
    uint32_t offset_default = 0;

    engine_file_write(0, "THUMBY_CLR_SAVE\n", 16);
    engine_file_write(0, &save_version, 2);
    engine_file_write(0, &data_length, 4);
    engine_file_write(0, &key_count, 2);
    for(uint16_t key_index; key_index<key_count; key_index++){
        engine_file_write(0, &offset_default, 4);
    }
}


void engine_save_set_file_location(const char *location, uint32_t len, uint16_t key_count){
    // Cannot be larger than this since `temp-` takes up 5 characters
    if(len > SAVE_LOCATION_LENGTH_MAX-5){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineSave: ERROR: location path too long (max length: 250)"));
    }

    // Set these equal to each other, for now
    current_location.len = len;
    temporary_location.len = len;

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

        engine_save_init_table(0, key_count);

        engine_file_close(0);
    }else{
        engine_file_open_read(0, &current_location);
        uint32_t file_size = engine_file_size(0);

        // char character
    }
}


void engine_save_del_set_location(){
    if(engine_file_exists(&current_location)){
        engine_file_remove(&current_location);
    } 
}


void engine_save_entry(const char* entry_name, uint32_t entry_len){
    engine_save_start_read_write();

    // uint32_t hash = qstr_compute_hash(entry_name, entry_len);
    // hash = hash % 

    engine_save_stop_read_write();
}


void engine_load_entry(){

}


void engine_load_entry_into(){

}


void engine_delete_entry(){

}