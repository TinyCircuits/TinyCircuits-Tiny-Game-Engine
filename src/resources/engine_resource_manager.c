#include "engine_resource_manager.h"
#include "debug/debug_print.h"
#include <math.h>


#ifndef __unix__
    #include "pico/stdlib.h"
    #include "hardware/flash.h"
    
    // The start of the portion of flash dedicated to aligned resources
    #define FLASH_RESOURCE_BASE_ADDRESS MICROPY_HW_FLASH_STORAGE_BYTES
    #define FLASH_RESOURCE_LENGTH (PICO_FLASH_SIZE_BYTES - XIP_BASE - MICROPY_HW_FLASH_STORAGE_BYTES) - FLASH_RESOURCE_BASE_ADDRESS
#endif


// The actual size of the portion of flash dedicated to aligned resources
uint32_t total_resource_space = 0;

// Where the next resource can go inside 'total_resource_space'
uint32_t free_room_start = 0;


void engine_resource_reset_resource_flash(){
    ENGINE_WARNING_PRINTF("TextureResource: Resetting resource flash space");

    #ifndef __unix__
        total_resource_space = FLASH_SECTOR_SIZE*(uint32_t)floor(FLASH_RESOURCE_LENGTH/FLASH_SECTOR_SIZE);
        free_room_start = 0;
        ENGINE_WARNING_PRINTF("TextureResource: Erasing %lu bytes starting at 0x%010x", total_resource_space, XIP_BASE+FLASH_RESOURCE_BASE_ADDRESS);
        flash_range_erase(FLASH_RESOURCE_BASE_ADDRESS, total_resource_space);
    #else
        ENGINE_INFO_PRINTF("TextureResource: Nothing to erase on this port...");
    #endif

    ENGINE_INFO_PRINTF("TextureResource: Done resetting resource flash space");
}