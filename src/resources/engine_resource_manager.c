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

    // Assuming the flash is partitioned such that the firmware
    // binary starts at XIP_BASE or the beginning of flash,
    // allow the firmware 1MiB of room.
    // PARTITION: | FIRMWARE | SCRATCH | FILESYSTEM |
    #define FLASH_RESOURCE_SPACE_BASE 1 * 1024 * 1024

    // The room left over after the room for the firmware
    // and the MicroPython filesystem is flash scratch
    #define FLASH_RESOURCE_SPACE_SIZE PICO_FLASH_SIZE_BYTES - (MICROPY_HW_FLASH_STORAGE_BYTES + FLASH_RESOURCE_SPACE_BASE)

    // Intermediate buffer to hold data read from flash before
    // programming it to a contigious flash area
    uint8_t page_prog[FLASH_PAGE_SIZE];
    uint16_t page_prog_index = 0;
    uint32_t page_prog_count = 0;

    // How many pages (not sectors), that have been used so far
    uint32_t used_pages_count = 0;
#endif


// These are used for tracking where we are storing
// data and where the data is stored inside that
// location
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


mp_obj_t engine_resource_get_space_bytearray(uint32_t space_size, bool fast_space){
    mp_obj_array_t *array = m_new_obj(mp_obj_array_t);
    array->base.type = &mp_type_bytearray;
    array->typecode = BYTEARRAY_TYPECODE;
    array->free = 0;
    array->len = space_size;

    if(fast_space){
        array->items = m_new(byte, array->len);
        memset(array->items, 0, array->len);
    }else{
        #ifdef __arm__
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

            // Before erasing everything for the requested size,
            // check if we're going to be erasing addresses out of
            // bounds, stop everything if that is going to happen
            // (will lose filesystem otherwise!)
            const uint32_t erase_size = sectors_to_erase_size;
            const uint32_t erase_start = FLASH_RESOURCE_SPACE_BASE + sectors_to_erase_offset;
            const uint32_t erase_end = erase_start + erase_size;

            if(erase_end > FLASH_RESOURCE_SPACE_BASE+FLASH_RESOURCE_SPACE_SIZE){
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineResourceManager: ERROR: Scratch space is going to overflow! Too many assets loaded! Scratch space is %ld bytes but the asset requires erasing %ld bytes from %ld to %ld"), FLASH_RESOURCE_SPACE_SIZE, erase_size, erase_start, erase_end);
            }
            
            // Need to disable interrupts when texture resources are created:
            // https://github.com/raspberrypi/pico-examples/issues/34#issuecomment-1369267917
            // otherwise hangs forever
            uint32_t paused_interrupts = save_and_disable_interrupts();
            flash_range_erase(erase_start, erase_size);
            restore_interrupts(paused_interrupts);

            // Stored in contiguous flash location
            array->items = (uint8_t*)(XIP_BASE + FLASH_RESOURCE_SPACE_BASE + (used_pages_count*FLASH_PAGE_SIZE));

            used_pages_count += required_pages_count;
        #endif
    }

    return array;
}


void engine_resource_start_storing(mp_obj_t bytearray, bool in_ram){
    current_storing_location = ENGINE_BYTEARRAY_OBJ_TO_DATA(bytearray);
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