#ifndef ENGINE_SAVE_H
#define ENGINE_SAVE_H

#include "py/obj.h"
#include <string.h>
#include <stdlib.h>


void engine_save_set_file_location(const char *location, uint32_t len, uint16_t key_count);
void engine_save_del_set_location();

void engine_save_entry(const char* entry_name, uint32_t entry_len);
void engine_load_entry();
void engine_load_entry_into();
void engine_delete_entry();


#endif  // ENGINE_SAVE_H