#include "py/obj.h"


// Module attributes
STATIC const mp_rom_map_elem_t engine_draw_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_draw) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_draw_globals, engine_draw_globals_table);

const mp_obj_module_t engine_draw_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_draw_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_draw, engine_draw_user_cmodule);