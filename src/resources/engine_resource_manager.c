#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "engine_resource_manager.h"
#include "debug/debug_print.h"
#include "py/obj.h"
#include "py/misc.h"
#include "utility/engine_file.h"



#ifndef __unix__
    #include "pico/stdlib.h"
    #include "hardware/flash.h"
    #include "hardware/sync.h"
    
    #define MICROPY_HW_FLASH_STORAGE_BASE (PICO_FLASH_SIZE_BYTES - MICROPY_HW_FLASH_STORAGE_BYTES)
    #define ENGINE_HW_FLASH_RESOURCE_SPACE_BASE MICROPY_HW_FLASH_STORAGE_BYTES

    // Intermediate buffer to hold data read from flash before
    // programming it to a contigious area
    uint8_t page_prog[FLASH_PAGE_SIZE];
    uint16_t page_prog_index = 0;
    uint32_t page_prog_count = 0;

    // How many pages (not sectors), that have been used so far
    uint32_t used_pages_count = 0;
#endif


uint8_t *current_storing_location = NULL;
uint32_t index_in_storing_location = 0;
bool storing_in_ram = false;


void engine_resource_reset(){
    ENGINE_PRINTF("EngineResourceManager: Resetting...\n");
    #if defined(__arm__)
        page_prog_index = 0;
        page_prog_count = 0;
        used_pages_count = 0;
        current_storing_location = NULL;
        index_in_storing_location = 0;
        storing_in_ram = false;
    #endif
}


// uint8_t *engine_resource_get_space_and_fill(const char *filename, uint32_t space_size, bool fast_space, uint32_t offset){
uint8_t *engine_resource_get_space(uint32_t space_size, bool fast_space){    
    uint8_t *space = NULL;

    #ifdef __unix__
        ENGINE_INFO_PRINTF("EngineResourceManager: Allocating ram for unix resource");
        space = malloc(space_size);
    #elif __arm__
        if(fast_space){
            ENGINE_INFO_PRINTF("EngineResourceManager: Allocating ram for rp3 resource");
            space = m_malloc(space_size);
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
            uint32_t already_erased_sectors_count = (uint32_t)ceil(((float)(used_pages_count*FLASH_PAGE_SIZE))/FLASH_SECTOR_SIZE);

            // Based on how many pages have been used and how many more are going to be used,
            // how many sectors should be erased
            uint32_t total_erase_sector_count = (uint32_t)ceil(((float)((required_pages_count+used_pages_count)*FLASH_PAGE_SIZE))/FLASH_SECTOR_SIZE);

            uint32_t additional_sectors_to_erase_count = total_erase_sector_count - already_erased_sectors_count;

            // Get the offset where sector erasing will start and
            // how many sectors, in bytes to erase, then erase them
            uint32_t sectors_to_erase_offset = already_erased_sectors_count*FLASH_SECTOR_SIZE;
            uint32_t sectors_to_erase_size   = additional_sectors_to_erase_count*FLASH_SECTOR_SIZE;

            ENGINE_INFO_PRINTF("ResourceManager: calculated for erasing and programming flash:");
            ENGINE_INFO_PRINTF("\trequired_pages_count:\t\t\t%lu", required_pages_count);
            ENGINE_INFO_PRINTF("\talready_erased_sectors_count:\t\t%lu", already_erased_sectors_count);
            ENGINE_INFO_PRINTF("\ttotal_erase_sector_count:\t\t%lu", total_erase_sector_count);
            ENGINE_INFO_PRINTF("\tadditional_sectors_to_erase_count:\t%lu", additional_sectors_to_erase_count);
            ENGINE_INFO_PRINTF("\tsectors_to_erase_offset:\t\t%lu", sectors_to_erase_offset);
            ENGINE_INFO_PRINTF("\tsectors_to_erase_size:\t\t\t%lu", sectors_to_erase_size);
            
            // Need to disable interrupts when texture resources are created:
            // https://github.com/raspberrypi/pico-examples/issues/34#issuecomment-1369267917
            // otherwise hangs forever
            uint32_t paused_interrupts = save_and_disable_interrupts();
            flash_range_erase(ENGINE_HW_FLASH_RESOURCE_SPACE_BASE+sectors_to_erase_offset, sectors_to_erase_size);
            restore_interrupts(paused_interrupts);

            // Stored in contiguous flash location
            space = (uint8_t*)(XIP_BASE + ENGINE_HW_FLASH_RESOURCE_SPACE_BASE + (used_pages_count*FLASH_PAGE_SIZE));

            used_pages_count += required_pages_count;
        }
    #else
        #error "EngineResourceManager: Unknown platform"
    #endif

    ENGINE_INFO_PRINTF("EngineResourceManager: Done allocating!");

    return space;
}


void engine_resource_start_storing(uint8_t *location, bool in_ram){
    current_storing_location = location;
    index_in_storing_location = 0;
    storing_in_ram = in_ram;

    // When using flash, need a way to track how many pages
    // in this storing operation have been stored, use this
    // to track
    #if defined(__arm__)
        page_prog_index = 0;
        page_prog_count = 0;
    #endif
}


void engine_resource_store_u8(uint8_t to_store){
    if(storing_in_ram){
        // Convert to u16 array
        uint8_t *u8_current_storing_location = (uint8_t*)current_storing_location;
        u8_current_storing_location[index_in_storing_location] = to_store;
    }else{
        #if defined(__arm__)
            uint8_t *u8_page_prog = (uint8_t*)page_prog;

            // Store the 'to_byte' byte in a buffer in ram for now
            u8_page_prog[page_prog_index] = to_store;

            // Once buffer is full, write it to flash and
            // reset indices to start filling again
            page_prog_index++;
            if(page_prog_index >= FLASH_PAGE_SIZE){
                uint32_t address_offset = ((uint32_t)current_storing_location) - XIP_BASE;
                uint32_t paused_interrupts = save_and_disable_interrupts();
                flash_range_program(address_offset + (page_prog_count*FLASH_PAGE_SIZE), page_prog, FLASH_PAGE_SIZE);
                restore_interrupts(paused_interrupts);

                page_prog_index = 0;
                page_prog_count++;
            }
        #else
            ENGINE_ERROR_PRINTF("EngineResourceManager: ERROR, no none ram programmer implemented on this platform! Resources will not work!");
        #endif
    }
    
    index_in_storing_location++;
}


void engine_resource_store_u16(uint16_t to_store){
    if(storing_in_ram){
        // Convert to u16 array
        uint16_t *u16_current_storing_location = (uint16_t*)current_storing_location;
        u16_current_storing_location[index_in_storing_location] = to_store;
    }else{
        #if defined(__arm__)
            uint16_t *u16_page_prog = (uint16_t*)page_prog;

            // Store the 'to_byte' byte in a buffer in ram for now
            u16_page_prog[page_prog_index] = to_store;

            // Once buffer is full, write it to flash and
            // reset indices to start filling again
            page_prog_index++;
            if(page_prog_index >= FLASH_PAGE_SIZE/2){
                uint32_t address_offset = ((uint32_t)current_storing_location) - XIP_BASE;
                uint32_t paused_interrupts = save_and_disable_interrupts();
                flash_range_program(address_offset + (page_prog_count*FLASH_PAGE_SIZE), page_prog, FLASH_PAGE_SIZE);
                restore_interrupts(paused_interrupts);

                page_prog_index = 0;
                page_prog_count++;
            }
        #else
            ENGINE_ERROR_PRINTF("EngineResourceManager: ERROR, no none ram programmer implemented on this platform! Resources will not work!");
        #endif
    }
    
    index_in_storing_location++;
}


void engine_resource_stop_storing(){
    #if defined(__arm__)
        if(page_prog_index != 0){
            uint32_t address_offset = ((uint32_t)current_storing_location) - XIP_BASE;
            uint32_t paused_interrupts = save_and_disable_interrupts();
            flash_range_program(address_offset + (page_prog_count*FLASH_PAGE_SIZE), page_prog, FLASH_PAGE_SIZE);
            restore_interrupts(paused_interrupts);
        }
    #endif
}