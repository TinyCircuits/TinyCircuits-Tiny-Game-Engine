#include "py/obj.h"
#include "engine_texture_resource.h"
#include "engine_wave_sound_resource.h"
#include "engine_tone_sound_resource.h"
#include "engine_font_resource.h"
#include "engine_noise_resource.h"
#include "engine_rtttl_sound_resource.h"
#include "engine_main.h"


static mp_obj_t engine_resources_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_resources_module_init_obj, engine_resources_module_init);    


/*  --- doc ---
    NAME: engine_resources
    ID: engine_resources
    DESC: Resources are objects that are used as references to certain data (textures/bitmaps, audio, fonts, etc.)
    ATTR: [type=object]   [name={ref_link:TextureResource}]     [value=object] 
    ATTR: [type=object]   [name={ref_link:WaveSoundResource}]   [value=object]
    ATTR: [type=object]   [name={ref_link:ToneSoundResource}]   [value=object]
    ATTR: [type=object]   [name={ref_link:FontResource}]        [value=object]
    ATTR: [type=object]   [name={ref_link:RTTTLSoundResource}]  [value=object]
*/
static const mp_rom_map_elem_t engine_resources_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_resources) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_resources_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_TextureResource), (mp_obj_t)&texture_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_WaveSoundResource), (mp_obj_t)&wave_sound_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ToneSoundResource), (mp_obj_t)&tone_sound_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_FontResource), (mp_obj_t)&font_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NoiseResource), (mp_obj_t)&noise_resource_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RTTTLSoundResource), (mp_obj_t)&rtttl_sound_resource_class_type },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_resources_globals, engine_resources_globals_table);

const mp_obj_module_t engine_resources_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_resources_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_resources, engine_resources_user_cmodule);