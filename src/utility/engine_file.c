#include "engine_file.h"
#include "debug/debug_print.h"


#if defined(__EMSCRIPTEN__)

#elif defined(__unix__)
    #include <stdlib.h>
    #include <fcntl.h>
    int file_fd;
#elif defined(__arm__)
    // See extmod/vfs_lfs.c + https://github.com/littlefs-project/littlefs
    #include "lib/littlefs/lfs2.h"
    #include "pico/stdlib.h"
    #include "hardware/flash.h"
    #include <math.h>
    #include <stdlib.h>

    #define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)
    #define ENGINE_HW_FLASH_SPRITE_SPACE_BASE MICROPY_HW_FLASH_STORAGE_BYTES

    lfs2_t littlefs2 = { 0 };
    lfs2_file_t littlefs2_file = { 0 };
    bool mounted = false;

    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L79-L90
    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
    int engine_lfs2_read(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, void *buffer, lfs2_size_t size){
        uint32_t offset = (block * FLASH_SECTOR_SIZE) + off;

        memcpy(buffer, (uint8_t *)(XIP_BASE + MICROPY_HW_FLASH_STORAGE_BASE + offset), size);

        return 0;
    }


    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L92-L115
    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L35 (BLOCK_SIZE)
    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
    int engine_lfs2_prog(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size){
        uint32_t offset = block * FLASH_SECTOR_SIZE;

        ENGINE_INFO_PRINTF("Engine File: Programming %lu bytes starting at %lu", size, offset);

        flash_range_erase(MICROPY_HW_FLASH_STORAGE_BASE + offset, size);
        flash_range_program(MICROPY_HW_FLASH_STORAGE_BASE + offset, buffer, size);

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
        flash_range_erase(MICROPY_HW_FLASH_STORAGE_BASE + offset, FLASH_SECTOR_SIZE);

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
#endif


void engine_file_open(const char *filename){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        file_fd = open(filename, O_RDONLY);
    #elif defined(__arm__)
        if(mounted == false){
            engine_file_mount();
            mounted = true;
        }

        // Need to use this cfg function since MicroPython is compiled without malloc (therefore need to supply)
        ENGINE_INFO_PRINTF("Engine File: Opening file '%s'...", filename);
        lfs2_file_opencfg(&littlefs2, &littlefs2_file, filename, LFS2_O_RDWR, &littlefs2_file_cfg);
        ENGINE_INFO_PRINTF("Engine File: Opening file '%s' complete!", filename);
    #endif
}


void engine_file_close(){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        close(file_fd);
    #elif defined(__arm__)
        ENGINE_INFO_PRINTF("Engine File: closing file...");
        lfs2_file_close(&littlefs2, &littlefs2_file);
        ENGINE_INFO_PRINTF("Engine File: closing file complete!");
    #endif
}


void engine_file_read(void *buffer, uint32_t size){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        read(file_fd, buffer, size);
    #elif defined(__arm__)
        lfs2_file_read(&littlefs2, &littlefs2_file, buffer, size);
    #endif
}


void engine_file_seek(uint32_t offset){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        lseek(file_fd, offset, SEEK_SET);
    #elif defined(__arm__)
        lfs2_file_seek(&littlefs2, &littlefs2_file, offset, LFS2_SEEK_SET);
    #endif
}


uint8_t engine_file_get_u8(uint32_t u8_byte_offset){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        uint8_t the_u8_byte = 0;
        lseek(file_fd, u8_byte_offset, SEEK_SET);
        read(file_fd, &the_u8_byte, 1);
        return the_u8_byte;
    #elif defined(__arm__)
        uint8_t the_u8_byte = 0;
        lfs2_file_seek(&littlefs2, &littlefs2_file, u8_byte_offset, LFS2_SEEK_SET);
        lfs2_file_read(&littlefs2, &littlefs2_file, &the_u8_byte, 1);
        return the_u8_byte;
    #endif
}


uint16_t engine_file_get_u16(uint32_t u8_byte_offset){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        uint16_t the_u16_byte = 0;
        lseek(file_fd, u8_byte_offset, SEEK_SET);
        read(file_fd, &the_u16_byte, 2);
        return the_u16_byte;
    #elif defined(__arm__)
        uint16_t the_u16_byte = 0;
        lfs2_file_seek(&littlefs2, &littlefs2_file, u8_byte_offset, LFS2_SEEK_SET);
        lfs2_file_read(&littlefs2, &littlefs2_file, &the_u16_byte, 2);
        return the_u16_byte;
    #endif
}


uint32_t engine_file_get_u32(uint32_t u8_byte_offset){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        uint32_t the_u32_byte = 0;
        lseek(file_fd, u8_byte_offset, SEEK_SET);
        read(file_fd, &the_u32_byte, 4);
        return the_u32_byte;
    #elif defined(__arm__)
        uint32_t the_u32_byte = 0;
        lfs2_file_seek(&littlefs2, &littlefs2_file, u8_byte_offset, LFS2_SEEK_SET);
        lfs2_file_read(&littlefs2, &littlefs2_file, &the_u32_byte, 4);
        return the_u32_byte;
    #endif
}


uint32_t engine_file_size(){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        return lseek(file_fd, 0, SEEK_END);
    #elif defined(__arm__)
        return lfs2_file_size(&littlefs2, &littlefs2_file);
    #endif
}