#include "py/obj.h"
#include "engine_texture_resource.h"


// Module attributes
STATIC const mp_rom_map_elem_t engine_resources_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_resources) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Vector2), (mp_obj_t)&texture_resource_class_type },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_resources_globals, engine_resources_globals_table);

const mp_obj_module_t engine_resources_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_resources_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_resources, engine_resources_user_cmodule);