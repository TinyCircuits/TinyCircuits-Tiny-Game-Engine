#ifndef ENGINE_FILE_H
#define ENGINE_FILE_H

#include "stdint.h"

void engine_file_open(const char *filename);

void engine_file_close();

void engine_file_read(void *buffer, uint32_t size);


#endif  // ENGINE_FILE_H