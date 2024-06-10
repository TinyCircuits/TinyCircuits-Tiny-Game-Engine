#ifndef ENGINE_SAVE_H
#define ENGINE_SAVE_H

#include "py/obj.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


void engine_saving_set_file_location(const byte* location, size_t location_len);
void engine_saving_del_set_location();

void engine_saving_save_entry(const byte* entry_name, size_t entry_name_len, mp_obj_t entry);
mp_obj_t engine_saving_load_entry(const byte* entry_name, size_t entry_name_len);
void engine_saving_delete_entry(const byte* entry_name, size_t entry_name_len);


#endif  // ENGINE_SAVE_H