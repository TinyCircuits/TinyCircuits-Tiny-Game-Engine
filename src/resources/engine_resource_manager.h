#ifndef ENGINE_RESOURCE_MANAGER_H
#define ENGINE_RESOURCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Erases continuous/aligned resource dedicated portion of flash 
void engine_resource_reset_resource_flash();

// Return pointer to space to store resources like sprite data, font, sound, etc.
// On the rp3 platform this can be flash or ram if fast. On unix, it will only
// ever be in ram
uint8_t *engine_resource_get_space_and_fill(const char *filename, uint32_t space_size, bool fast_space, uint32_t offset);

#endif  // ENGINE_RESOURCE_MANAGER_H