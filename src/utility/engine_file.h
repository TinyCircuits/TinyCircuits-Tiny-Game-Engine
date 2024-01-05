#ifndef ENGINE_FILE_RP2_H
#define ENGINE_FILE_RP2_H

#include <stdint.h>
#include <stdbool.h>


// Opening and seeking around files using UNIX or LittleFS
// operations is slow! These structures and related functions
// are used to store information about the location of data
// in the file for faster access. On UNIX it actually
// just loads the entire file contents into ram. On
// LittleFS platforms, it stores flash offsets to LittleFS
// blocks that it quickly uses to index into the fragmented
// locations and get the data directly from flash. This means
// the flash bus is receiving at least 4 bytes (offset) for every 
// 2 from flash when getting pixels
typedef struct engine_fast_cache_file{
    uint8_t *file_data;
    uint32_t file_size;
    bool in_ram;
}engine_fast_cache_file_t;

// #if defined(__unix__)
//     typedef struct engine_fast_cache_file{
//         uint8_t *file_data;
//     }engine_fast_cache_file_t;
// #elif defined (__arm__)
//     typedef struct engine_fast_cache_file{
//         uint32_t *block_flash_addresses;
//     }engine_fast_cache_file_t;
// #else
//     #error "Engine File: Unsupported platform!"
// #endif


// Read/store information about the file at 'filename'
void engine_fast_cache_file_init(engine_fast_cache_file_t *cache_file, const char *filename);

void engine_fast_cache_file_deinit(engine_fast_cache_file_t *cache_file);

// Get a single byte from the cached/inited file
uint8_t engine_fast_cache_file_get_u8(engine_fast_cache_file_t *cache_file, uint32_t offset_u8);

// Get two bytes as uint16_t from file
uint16_t engine_fast_cache_file_get_u16(engine_fast_cache_file_t *cache_file, uint32_t offset_u16);


// // Open a file instance until it is closed (cannot use this
// // across the engine to open multiple files at the same time)
// void engine_file_open(const char *filename);

// // Close the file opened by 'engine_file_open(...)' (need to close
// // files on same thread in sequence with open, cannot be used across
// // the engine handle multiple files at the same time)
// void engine_file_close();


// void engine_file_read(void *buffer, uint32_t size);
// uint8_t engine_file_get_u8(uint32_t u8_byte_offset);
// uint16_t engine_file_get_u16(uint32_t u16_byte_offset);

#endif  // ENGINE_FILE_RP2_H