#ifndef ENGINE_FILE_RP2_H
#define ENGINE_FILE_RP2_H

#include <stdint.h>
#include <stdbool.h>
#include "py/objstr.h"

// Open a file instance until it is closed (cannot use this
// across the engine to open multiple files at the same time)
void engine_file_open_read(uint8_t file_index, mp_obj_str_t *filename);
void engine_file_open_create_write(uint8_t file_index, mp_obj_str_t *filename);

// Close the file opened by 'engine_file_open(...)' (need to close
// files on same thread in sequence with open, cannot be used across
// the engine handle multiple files at the same time)
void engine_file_close(uint8_t file_index);

void engine_file_read(uint8_t file_index, void *buffer, uint32_t size);
void engine_file_write(uint8_t file_index, void *buffer, uint32_t size);
void engine_file_seek(uint8_t file_index, uint32_t offset);
uint8_t engine_file_get_u8(uint8_t file_index, uint32_t u8_byte_offset);
uint16_t engine_file_get_u16(uint8_t file_index, uint32_t u8_byte_offset);
uint32_t engine_file_get_u32(uint8_t file_index, uint32_t u8_byte_offset);

uint32_t engine_file_size(uint8_t file_index);

void engine_file_remove(mp_obj_str_t *filename);
void engine_file_rename(mp_obj_str_t *old, mp_obj_str_t *new);
bool engine_file_exists(mp_obj_str_t *filename);

#endif  // ENGINE_FILE_RP2_H