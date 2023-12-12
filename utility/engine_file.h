#ifndef ENGINE_FILE_H
#define ENGINE_FILE_H

#include "stdint.h"

void engine_file_open(const char *filename);

uint32_t engine_file_get_size(const char *filename);

void engine_file_close();

void engine_file_read(void *buffer, uint32_t size);

uint8_t engine_file_get_u8(uint32_t u8_byte_offset);
uint16_t engine_file_get_u16(uint32_t u16_byte_offset);

#endif  // ENGINE_FILE_H