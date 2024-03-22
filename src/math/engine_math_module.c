#include "py/obj.h"

#include "vector2.h"
#include "vector3.h"
#include "rectangle.h"


/*  --- doc ---
    NAME: engine_math
    DESC: Module for common math operations and objects
    ATTR: [type=object]   [name={ref_link:Vector2}]     [value=object]
    ATTR: [type=object]   [name={ref_link:Vector3}]     [value=object]
    ATTR: [type=object]   [name={ref_link:Rectangle}]   [value=object]
*/
STATIC const mp_rom_map_elem_t engine_math_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_math) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Vector2), (mp_obj_t)&vector2_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Vector3), (mp_obj_t)&vector3_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Rectangle), (mp_obj_t)&rectangle_class_type },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_math_globals, engine_math_globals_table);

const mp_obj_module_t engine_math_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_math_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_math, engine_math_user_cmodule);