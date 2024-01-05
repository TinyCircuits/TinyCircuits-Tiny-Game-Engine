#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "engine_resource_manager.h"
#include "debug/debug_print.h"
#include "py/obj.h"
#include "utility/engine_file.h"


#ifndef __unix__
    #include "pico/stdlib.h"
    #include "hardware/flash.h"
    
    #define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)
    #define ENGINE_HW_FLASH_SPRITE_SPACE_BASE MICROPY_HW_FLASH_STORAGE_BYTES

    // Intermediate buffer to hold data read from flash before
    // programming it to a contigious area
    uint8_t page_prog[FLASH_PAGE_SIZE];

    // How many pages (not sectors), that have been used so far
    uint32_t used_pages = 0;

    // Count of how many sectors have been erased (not pages)
    uint32_t erased_sectors = 0;
#endif


uint8_t *engine_resource_get_space_and_fill(const char *filename, uint32_t space_size, bool fast_space, uint32_t offset){
    engine_file_open(filename);
    engine_file_seek(offset);
    uint8_t *space;

    #ifdef __unix__
        ENGINE_INFO_PRINTF("EngineResourceManager: Allocating ram for unix resource");
        space = (uint8_t*)malloc(space_size);
        engine_file_read(space, space_size);
    #elif __arm__
        if(fast_space){
            ENGINE_INFO_PRINTF("EngineResourceManager: Allocating ram for rp3 resource");
            space = (uint8_t*)m_malloc(space_size);    // Will this get collected by gc?
            engine_file_read(space, space_size);
        }else{
            ENGINE_INFO_PRINTF("EngineResourceManager: Allocating flash for rp3 resource");

            uint32_t required_pages = (uint32_t)ceil(space_size/FLASH_PAGE_SIZE);
            uint32_t erased_sectors = (uint32_t)floor((used_pages)/FLASH_SECTOR_SIZE);
            uint32_t erased_sectors_offset = erased_sectors * FLASH_SECTOR_SIZE;
            uint32_t required_sectors = (uint32_t)ceil((float)(used_pages+required_pages)/(float)FLASH_SECTOR_SIZE);
            uint32_t sectors_left_to_erase = required_sectors-erased_sectors;

            // Erase sectors before programming them
            flash_range_erase(ENGINE_HW_FLASH_SPRITE_SPACE_BASE+erased_sectors_offset, sectors_left_to_erase*FLASH_SECTOR_SIZE);

            for(uint32_t ipx=0; ipx<required_pages; ipx++){
                engine_file_read(page_prog, FLASH_PAGE_SIZE);
                flash_range_program(ENGINE_HW_FLASH_SPRITE_SPACE_BASE+(used_pages*FLASH_PAGE_SIZE)+(ipx*FLASH_PAGE_SIZE), page_prog, FLASH_PAGE_SIZE);
            }

            // Stored in contiguous flash location
            space = (uint8_t*)(XIP_BASE + ENGINE_HW_FLASH_SPRITE_SPACE_BASE + (used_pages*FLASH_PAGE_SIZE));

            used_pages += required_pages;
        }
    #else
        #error "EngineResource: Unknown platform"
    #endif

    ENGINE_INFO_PRINTF("EngineResourceManager: Done allocating!");

    engine_file_close();

    return space;
}