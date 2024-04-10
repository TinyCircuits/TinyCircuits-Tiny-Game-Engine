#include "py/obj.h"
#include "engine_texture_resource.h"
#include "engine_wave_sound_resource.h"
#include "engine_font_resource.h"
#include "engine_noise_resource.h"


/*  --- doc ---
    NAME: engine_resources
    DESC: Resources are objects that are used as references to certain data (textures/bitmaps, audio, fonts, etc.)
    ATTR: [type=object]   [name={ref_link:TextureResource}]     [value=object] 
    ATTR: [type=object]   [name={ref_link:WaveSoundResource}]   [value=object]
    ATTR: [type=object]   [name={ref_link:FontResource}]        [value=object]
*/
STATIC const mp_rom_map_elem_t engine_resources_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_resources) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_TextureResource), (mp_obj_t)&texture_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WaveSoundResource), (mp_obj_t)&wave_sound_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FontResource), (mp_obj_t)&font_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NoiseResource), (mp_obj_t)&noise_resource_class_type },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_resources_globals, engine_resources_globals_table);

const mp_obj_module_t engine_resources_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_resources_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_resources, engine_resources_user_cmodule);