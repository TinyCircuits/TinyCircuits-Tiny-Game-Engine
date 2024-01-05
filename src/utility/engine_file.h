#ifndef ENGINE_FILE_RP2_H
#define ENGINE_FILE_RP2_H

#include <stdint.h>
#include <stdbool.h>

// Open a file instance until it is closed (cannot use this
// across the engine to open multiple files at the same time)
void engine_file_open(const char *filename);

// Close the file opened by 'engine_file_open(...)' (need to close
// files on same thread in sequence with open, cannot be used across
// the engine handle multiple files at the same time)
void engine_file_close();

void engine_file_read(void *buffer, uint32_t size);
void engine_file_seek(uint32_t offset);
uint8_t engine_file_get_u8(uint32_t u8_byte_offset);
uint16_t engine_file_get_u16(uint32_t u8_byte_offset);
uint32_t engine_file_get_u32(uint32_t u8_byte_offset);

uint32_t engine_file_size();

#endif  // ENGINE_FILE_RP2_H