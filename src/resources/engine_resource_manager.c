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
    #include "hardware/sync.h"
    
    #define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)
    #define ENGINE_HW_FLASH_SPRITE_SPACE_BASE MICROPY_HW_FLASH_STORAGE_BYTES

    // Intermediate buffer to hold data read from flash before
    // programming it to a contigious area
    uint8_t page_prog[FLASH_PAGE_SIZE];

    // How many pages (not sectors), that have been used so far
    uint32_t used_pages_count = 0;
#endif


uint8_t *engine_resource_get_space_and_fill(const char *filename, uint32_t space_size, bool fast_space, uint32_t offset){
    engine_file_open(filename);
    engine_file_seek(offset);
    uint8_t *space = NULL;

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

            // How many flash pages will be needed to fit 'space_size' data? 
            // Pages are 256 bytes and data must be written in that page size:
            // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#rpip8ee511575881aa0f3936
            uint32_t required_pages_count = (uint32_t)ceil(((float)space_size)/FLASH_PAGE_SIZE);

            // Based on how many pages have been used so far, how many sectors have
            // already been erased? This will be used to find the base offset of sectors
            // to erase if the extra pages end up in new sectors. Sectors are 4096 bytes
            // and must be erased in that sector size:
            // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#rpip8ee511575881aa0f3936
            uint32_t already_erased_sectors_count = (uint32_t)ceil(((float)used_pages_count)/FLASH_SECTOR_SIZE);

            // Based on how many pages have been used and how many more are going to be used,
            // how many sectors should be erased
            uint32_t total_erase_sector_count = (uint32_t)ceil(((float)(required_pages_count+used_pages_count))/FLASH_SECTOR_SIZE);

            uint32_t additional_sectors_to_erase_count = total_erase_sector_count - already_erased_sectors_count;

            // Get the offset where sector erasing will start and
            // how many sectors, in bytes to erase, then erase them
            uint32_t sectors_to_erase_offset = already_erased_sectors_count*FLASH_SECTOR_SIZE;
            uint32_t sectors_to_erase_size   = additional_sectors_to_erase_count*FLASH_SECTOR_SIZE;
            
            // Need to disable interrupts when texture resources are created:
            // https://github.com/raspberrypi/pico-examples/issues/34#issuecomment-1369267917
            // otherwise hangs forever
            uint32_t paused_interrupts = save_and_disable_interrupts();
            flash_range_erase(ENGINE_HW_FLASH_SPRITE_SPACE_BASE+sectors_to_erase_offset, sectors_to_erase_size);

            for(uint32_t ipx=0; ipx<required_pages_count; ipx++){
                engine_file_read(page_prog, FLASH_PAGE_SIZE);
                flash_range_program(ENGINE_HW_FLASH_SPRITE_SPACE_BASE+(used_pages_count*FLASH_PAGE_SIZE)+(ipx*FLASH_PAGE_SIZE), page_prog, FLASH_PAGE_SIZE);
            }
            restore_interrupts(paused_interrupts);

            // Stored in contiguous flash location
            space = (uint8_t*)(XIP_BASE + ENGINE_HW_FLASH_SPRITE_SPACE_BASE + (used_pages_count*FLASH_PAGE_SIZE));

            used_pages_count += required_pages_count;
        }
    #else
        #error "EngineResource: Unknown platform"
    #endif

    ENGINE_INFO_PRINTF("EngineResourceManager: Done allocating!");

    engine_file_close();

    return space;
}