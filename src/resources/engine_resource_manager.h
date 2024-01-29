#ifndef ENGINE_RESOURCE_MANAGER_H
#define ENGINE_RESOURCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Return pointer to space to store resources like sprite data, font, sound, etc.
// On the rp3 platform this can be flash or ram if fast. On unix, it will only
// ever be in ram
uint8_t *engine_resource_get_space(uint32_t space_size, bool fast_space);

// Because the RP3 port requires that flash be programmed in 256
// sized blocks, define functions to serially store data in a
// resource location. All platforms should serially load assets
// in to resource space locations since RP3 port has to anyway
void engine_resource_start_storing(uint8_t *location, bool in_ram);

// No offset since expects that data will be stored serially
// to allow constrained embedded devices to program flash
// in a non-random uniform manner
void engine_resource_store_u8(uint8_t to_store);
void engine_resource_store_u16(uint16_t to_store);

// Need to call this to push the remaining potentially partially
// intermediate buffer how to flash (in the case of embedded non-ram locations)
void engine_resource_stop_storing();

#endif  // ENGINE_RESOURCE_MANAGER_H