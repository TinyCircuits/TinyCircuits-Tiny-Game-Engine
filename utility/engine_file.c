#include "engine_file.h"
#include "debug/debug_print.h"

#if defined(__unix__)

#elif defined(__arm__)
    // See extmod/vfs_lfs.c + https://github.com/littlefs-project/littlefs
    #include "lib/littlefs/lfs2.h"

    #include "pico/stdlib.h"
    #include "hardware/flash.h"

    #define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)
#else
    #error "EngineFile: Unsupported platform, engine file operations will not work"
#endif




lfs2_t littlefs2;
lfs2_file_t littlefs2_file;
bool mounted = false;


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L79-L90
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_read(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t offset, void *buffer, lfs2_size_t size){
    return memcpy(buffer, (void *)(XIP_BASE + MICROPY_HW_FLASH_STORAGE_BASE + offset), size);
}


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L92-L115
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L35 (BLOCK_SIZE)
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_prog(const struct lfs2_config *c, lfs2_block_t block, lfs2_off_t off, const void *buffer, lfs2_size_t size){
    uint32_t offset = block * FLASH_SECTOR_SIZE;

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
    return 0;
}


// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L131
// https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L56C19-L56C48 (flash_base)
int engine_lfs2_erase(const struct lfs2_config *c, lfs2_block_t block){
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
const struct lfs2_config littlefs2_cfg = {
    // block device operations
    .read  = engine_lfs2_read,
    .prog  = engine_lfs2_prog,
    .sync  = engine_lfs2_sync,
    .erase = engine_lfs2_erase,

    // block device configuration
    .read_size = 32,
    .prog_size = 256,
    .lookahead_size = 32,

    .block_size = FLASH_SECTOR_SIZE,                                    // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L129-L130
    .block_count = MICROPY_HW_FLASH_STORAGE_BYTES / FLASH_SECTOR_SIZE,  // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/ports/rp2/rp2_flash.c#L127C44-L128
    .cache_size = 4*256,                                                // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L95
    .block_cycles = 100,                                                // https://github.com/TinyCircuits/micropython/blob/9b486340da22931cde82872f79e1c34db959548b/extmod/vfs_lfsx.c#L94
};



// NOTE: see comments above mount function in extmod/lfs2.h.
// Multiple mounts are allowed at the same time, so if MicroPython
// did a mount somewhere else that should be perfectly fine
// https://github.com/littlefs-project/littlefs/tree/master
void engine_file_mount(){
    if(lfs2_mount(&littlefs2, &littlefs2_cfg)){
        ENGINE_ERROR_PRINTF("Engine File: Something went wrong while mounting the filesystem...");
    }
}


void engine_file_open(const char *filename, int flags){
    if(mounted == false){
        engine_file_mount();
        mounted=true;
    }

    lfs2_file_open(&littlefs2, &littlefs2_file, filename, flags);
}


void engine_file_close(){
    lfs2_file_close(&littlefs2, &littlefs2_file);
}


void engine_file_read(void *buffer, uint32_t size){
    lfs2_file_read(&littlefs2, &littlefs2_file, buffer, size);
}