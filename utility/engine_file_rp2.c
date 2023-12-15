#include "engine_file.h"
#include "debug/debug_print.h"


// Because of the vast differences between file operations on RP2 and UNIX,
// there are separate translation units (.c files) for each platform.


// See extmod/vfs_lfs.c + https://github.com/littlefs-project/littlefs
#include "lib/littlefs/lfs2.h"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <math.h>

#define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)



lfs2_t littlefs2 = { 0 };
lfs2_file_t littlefs2_file = { 0 };
bool mounted = false;

uint32_t *current_block_addresses;
uint32_t current_block_count = 0;
bool track_blocks = false;


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L79-L90
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_read(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, uint8_t *buffer, lfs2_size_t size){
    uint32_t offset = (block * FLASH_SECTOR_SIZE) + off;

    ENGINE_INFO_PRINTF("Engine File RP2: Block %lu %lu %lu", block, off, size);

    if(track_blocks){
        if(current_block_count == 0){
            current_block_addresses[0] = block;
            current_block_count = 1;
            // ENGINE_INFO_PRINTF("Engine File RP2: Tracking block %lu", block);
        }else{
            bool already_tracked = false;
            for(uint32_t ibx=0; ibx<current_block_count; ibx++){
                if(current_block_addresses[ibx] == block){
                    already_tracked = true;
                    break;
                }
            }

            if(!already_tracked){
                current_block_addresses[current_block_count] = block;
                current_block_count++;
                // ENGINE_INFO_PRINTF("Engine File RP2: Tracking block %lu", block);
            }
            
        }
    }else{
        // ENGINE_INFO_PRINTF("Engine File RP2: Reading %lu bytes starting at %lu", size, offset);
    }

    memcpy(buffer, (uint8_t *)(XIP_BASE + MICROPY_HW_FLASH_STORAGE_BASE + offset), size);

    return 0;
}


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L92-L115
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L35 (BLOCK_SIZE)
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_prog(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size){
    uint32_t offset = block * FLASH_SECTOR_SIZE;

    ENGINE_INFO_PRINTF("Engine File: Programming %lu bytes starting at %lu", size, offset);

    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    flash_range_erase(MICROPY_HW_FLASH_STORAGE_BASE + offset, size);
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    MICROPY_EVENT_POLL_HOOK

    // Flash erase/program must run in an atomic section because the XIP bit gets disabled.
    atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    flash_range_program(MICROPY_HW_FLASH_STORAGE_BASE + offset, buffer, size);
    MICROPY_END_ATOMIC_SECTION(atomic_state);
    MICROPY_EVENT_POLL_HOOK

    return 0;
}


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L125
int engine_lfs2_sync(const struct lfs2_config *c){
    ENGINE_INFO_PRINTF("Engine File: Sync (does nothing)");
    return 0;
}


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L131
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_erase(const struct lfs2_config *c, lfs2_block_t block){
    ENGINE_INFO_PRINTF("Engine File: Erasing block %lu", block);

    uint32_t offset = block * FLASH_SECTOR_SIZE;
    // Flash erase/program must run in an atomic section because the XIP bit gets disabled.
    mp_uint_t atomic_state = MICROPY_BEGIN_ATOMIC_SECTION();
    flash_range_erase(MICROPY_HW_FLASH_STORAGE_BASE + offset, FLASH_SECTOR_SIZE);
    MICROPY_END_ATOMIC_SECTION(atomic_state);

    return 0;
}


// Quite the rabbit hole to figure out the parameters MicroPython uses for littlefs operations:
// 1. Block device and filesystem parameters flow starts in Python at boot: https://github.com/TinyCircuits/micropython/blob/master/ports/rp2/modules/_boot.py#L7-L13
// 2. The 'Block Device' is really just a structure that holds fs start and size in flash: https://github.com/TinyCircuits/micropython/blob/master/ports/rp2/rp2_flash.c#L54-L58
// 3. The 'VfsLfs2' QSTR is here which does the constructor that starts up most of the filesystem: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/moduos.c#L165
// 4. That type is redefined here: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfs.c#L101
// 5. Which means its make_new function is really here: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L116-L134
// 6. The 'init_config' (which we're doing here) is passed parameters: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L127-L128
// 7. Those parameters are indexed through enums to here: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfs.c#L37-L43
// From '7' we can see:
//      read_size = 32
//      prog_size = 256 (see _boot.py)
//      lookahead = 32
// 8. We still need more information, that can be found here: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L64
// 9. The 'read' and 'prog' callbacks seem to be defined here: https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L79-L115
#define ENGINE_LFS2_PROG_SIZE 32
#define ENGINE_LFS2_LOOKAHEAD_SIZE 32
#define ENGINE_LFS2_CACHE_SIZE 2 * ENGINE_LFS2_PROG_SIZE


// LittleFS stores all file data in blocks. Those blocks can be metapairs or CTZ blocks.
// Metapair blocks store information about the entire file while also potentially storing
// all the file data if the file is small enough. CTZ blocks store linked list data
// as well as file data afterwards


// MicroPython compiles its LFS2 with no malloc so we need to make our own buffers
uint8_t engine_lfs2_read_buffer[ENGINE_LFS2_CACHE_SIZE];
uint8_t engine_lfs2_prog_buffer[ENGINE_LFS2_CACHE_SIZE];
uint8_t engine_lfs2_lookahead_buffer[ENGINE_LFS2_LOOKAHEAD_SIZE];
uint8_t engine_lfs2_file_buffer[ENGINE_LFS2_CACHE_SIZE];


const struct lfs2_config littlefs2_cfg = {
    // block device operations
    .read  = engine_lfs2_read,
    .prog  = engine_lfs2_prog,
    .sync  = engine_lfs2_sync,
    .erase = engine_lfs2_erase,

    // block device configuration
    .read_size = 32,
    .prog_size = ENGINE_LFS2_PROG_SIZE,
    .lookahead_size = ENGINE_LFS2_LOOKAHEAD_SIZE,

    .block_size = FLASH_SECTOR_SIZE,                                    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L129-L130
    .block_count = MICROPY_HW_FLASH_STORAGE_BYTES / FLASH_SECTOR_SIZE,  // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L127C44-L128
    .cache_size = ENGINE_LFS2_CACHE_SIZE,                               // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L95
    .block_cycles = 100,                                                // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L94

    .read_buffer = engine_lfs2_read_buffer,
    .prog_buffer = engine_lfs2_prog_buffer,
    .lookahead_buffer = engine_lfs2_lookahead_buffer
};


const struct lfs2_file_config littlefs2_file_cfg = {
    .buffer = engine_lfs2_file_buffer
};



// NOTE: see comments above mount function in extmod/lfs2.h.
// Multiple mounts are allowed at the same time, so if MicroPython
// did a mount somewhere else that should be perfectly fine
// https://github.com/littlefs-project/littlefs/tree/master
void engine_file_mount(){
    ENGINE_INFO_PRINTF("Engine File: Mounting filesystem...");

    int err = lfs2_mount(&littlefs2, &littlefs2_cfg);

    if(err < 0){
        ENGINE_ERROR_PRINTF("Engine File: Something went wrong while mounting the filesystem... error %d", err);
    }

    ENGINE_INFO_PRINTF("Engine File: Mounting filesystem complete!");
}


void engine_file_open(const char *filename){
    if(mounted == false){
        engine_file_mount();
        mounted = true;
    }

    // Need to use this function since MicroPython is compiled without malloc (therefore need to supply)
    ENGINE_INFO_PRINTF("Engine File: Opening file '%s'...", filename);
    lfs2_file_opencfg(&littlefs2, &littlefs2_file, filename, LFS2_O_RDWR, &littlefs2_file_cfg);
    ENGINE_INFO_PRINTF("Engine File: Opening file '%s' complete!", filename);
}


void engine_file_close(){
    ENGINE_INFO_PRINTF("Engine File: closing file...");
    lfs2_file_close(&littlefs2, &littlefs2_file);
    ENGINE_INFO_PRINTF("Engine File: closing file complete!");
}


void engine_file_read(void *buffer, uint32_t size){
    lfs2_file_read(&littlefs2, &littlefs2_file, buffer, size);
}


uint8_t engine_file_get_u8(uint32_t u8_byte_offset){
    uint8_t the_u8_byte = 0;
    lfs2_file_seek(&littlefs2, &littlefs2_file, u8_byte_offset, LFS2_SEEK_SET);
    lfs2_file_read(&littlefs2, &littlefs2_file, &the_u8_byte, 1);
    return the_u8_byte;
}


uint16_t engine_file_get_u16(uint32_t u16_byte_offset){
    uint16_t the_u16_byte = 0;
    lfs2_file_seek(&littlefs2, &littlefs2_file, u16_byte_offset*2, LFS2_SEEK_SET);
    lfs2_file_read(&littlefs2, &littlefs2_file, &the_u16_byte, 2);
    return the_u16_byte;
}


// Comparison function for qsort
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}


void engine_fast_cache_file_init(engine_fast_cache_file_t *cache_file, const char *filename){
    // In case the file system has not been mounted yet, mount it
    if(mounted == false){
        engine_file_mount();
        mounted = true;
    }

    // Open file that we want to cache information about
    // engine_file_open(filename);


    // engine_file_open("Caching file...");
    // engine_file_open("'%s' information:", filename);

    // ENGINE_WARNING_PRINTF("%lu", littlefs2_file.ctz.head);


    // engine_file_open("Caching file complete!");


    // uint32_t file_size = lfs2_file_size(&littlefs2, &littlefs2_file);


    // // Get file size to see how many blocks the file
    // // will occupy
    // struct lfs2_info littlefs2_file_info = { 0 };
    // lfs2_stat(&littlefs2, filename, &littlefs2_file_info);
    // uint32_t file_size = littlefs2_file_info.size;
    // uint32_t block_count = (uint32_t)ceil(((float)file_size)/FLASH_SECTOR_SIZE);
    
    // // Allocate enough space to store all the addresses
    // cache_file->block_flash_addresses = (uint32_t*)malloc(sizeof(uint32_t)*block_count);

    // engine_file_open(filename);


    // current_block_addresses = cache_file->block_flash_addresses;
    // current_block_count = 0;
    // track_blocks = true;

    // uint8_t data = 0;
    // ENGINE_INFO_PRINTF("Engine File RP2: Reading byte by byte...");
    // while(lfs2_file_read(&littlefs2, &littlefs2_file, &data, 1) != 0){}
    // ENGINE_INFO_PRINTF("Engine File RP2: Reading byte by byte complete!");
    // track_blocks = false;

    // qsort(cache_file->block_flash_addresses, block_count, sizeof(int), compare);

    // for(uint32_t ibx=0; ibx<block_count; ibx++){
    //     ENGINE_INFO_PRINTF("Engine File RP2: Sorted blocks: %lu [index: %lu]", cache_file->block_flash_addresses[ibx], ibx);
    // }


    // engine_file_close();

    // ENGINE_INFO_PRINTF("Engine File RP2: Block count %lu %lu", block_count, file_size);
}


void engine_fast_cache_file_deinit(engine_fast_cache_file_t *cache_file){
    free(cache_file->block_flash_addresses);
}


uint8_t engine_fast_cache_file_get_u8(engine_fast_cache_file_t *cache_file, uint32_t offset_u8){
    
}


uint16_t engine_fast_cache_file_get_u16(engine_fast_cache_file_t *cache_file, uint32_t offset_u16){
    return 0xaaaa;
    // uint32_t offset_u8 = offset_u16*2;

    // uint32_t block_index = (uint32_t)floor((offset_u8) / FLASH_SECTOR_SIZE);
    // // ENGINE_ERROR_PRINTF("%lu", block_index);

    // uint32_t offset_inside_block = offset_u8-(block_index*FLASH_SECTOR_SIZE);
    // // ENGINE_ERROR_PRINTF("%lu", offset_inside_block);

    // uint32_t flash_block_index = cache_file->block_flash_addresses[block_index];
    // // ENGINE_ERROR_PRINTF("%lu", flash_block_index);

    // uint32_t flash_location = (flash_block_index*FLASH_SECTOR_SIZE)+offset_inside_block;
    // // ENGINE_ERROR_PRINTF("%lu\n", flash_location);

    // const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + MICROPY_HW_FLASH_STORAGE_BASE + flash_location);

    // uint16_t u16 = 0;
    // u16 = u16 | flash_target_contents[1] << 8;
    // u16 = u16 | flash_target_contents[0];

    // return u16;
}