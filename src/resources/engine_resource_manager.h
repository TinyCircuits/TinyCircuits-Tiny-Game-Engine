#ifndef ENGINE_RESOURCE_MANAGER_H
#define ENGINE_RESOURCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#include "py/obj.h"
#include "py/misc.h"
#include "py/binary.h"
#include "py/objarray.h"

#define ENGINE_BYTEARRAY_OBJ_TO_DATA(bytearray) ((mp_obj_array_t*)bytearray)->items
#define ENGINE_BYTEARRAY_OBJ_LEN(bytearray) ((mp_obj_array_t*)bytearray)->len

void engine_resource_init();

// Resets counters and positions so that assets can be written to flash
// from the start, again
void engine_resource_reset();

// Return pointer to space to store resources like sprite data, font, sound, etc.
// On the rp3 platform this can be flash or ram if fast. On unix, it will only
// ever be in ram
mp_obj_t engine_resource_get_space_bytearray(uint32_t space_size, bool fast_space);

// Because the RP3 port requires that flash be programmed in 256
// sized blocks, define functions to serially store data in a
// resource location. All platforms should serially load assets
// in to resource space locations since RP3 port has to anyway
void engine_resource_start_storing(mp_obj_t bytearray, bool in_ram);

// No offset since expects that data will be stored serially
// to allow constrained embedded devices to program flash
// in a non-random uniform manner
void engine_resource_store_u8(uint8_t to_store);
void engine_resource_store_u16(uint16_t to_store);

// Need to call this to push the remaining potentially partially
// intermediate buffer how to flash (in the case of embedded non-ram locations)
void engine_resource_stop_storing();

#endif  // ENGINE_RESOURCE_MANAGER_H