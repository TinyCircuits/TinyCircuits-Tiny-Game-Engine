#include "py/obj.h"
#include "engine_utilities_module.h"
#include "debug/debug_print.h"
#include "math/engine_math.h"
#include "math/vector2.h"


/* --- doc ---
   NAME: generate_polygon_rectangle
   DESC: Generates a list of {ref_link:Vector2}s that represent the clockwise ordered shape of a rectangle of 4 points.
   PARAM:  [type=float]         [name=nothing, scale, or width and height]                         [value=float]
   RETURN: List of {ref_link:Vector2}s
*/
STATIC mp_obj_t engine_utilities_generate_polygon_rectangle(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("EngineUtilities: Generating Polygon Rectangle");

    if(n_args == 0){
        return mp_obj_new_list(4, (mp_obj_t[]){ vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f), mp_obj_new_float(1.0f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f),  mp_obj_new_float(1.0f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f),  mp_obj_new_float(-1.0f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f), mp_obj_new_float(-1.0f)})});
    }else if(n_args == 1){
        float scale = mp_obj_get_float(args[0]);
        return mp_obj_new_list(4, (mp_obj_t[]){ vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f*scale), mp_obj_new_float(1.0f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f*scale),  mp_obj_new_float(1.0f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f*scale),  mp_obj_new_float(-1.0f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f*scale), mp_obj_new_float(-1.0f*scale)})});
    }else if(n_args == 2){
        float width = mp_obj_get_float(args[0]);
        float height = mp_obj_get_float(args[1]);
        return mp_obj_new_list(4, (mp_obj_t[]){ vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-width/2), mp_obj_new_float(height/2)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(width/2),  mp_obj_new_float(height/2)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(width/2),  mp_obj_new_float(-height/2)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-width/2), mp_obj_new_float(-height/2)})});
    }
    
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_utilities_generate_polygon_rectangle_obj, 0, 2, engine_utilities_generate_polygon_rectangle);


/* --- doc ---
    NAME: generate_polygon_hexagon
    DESC: Generates a list of {ref_link:Vector2}s that represent the clockwise ordered shape of a hexagon of 6 points.
    PARAM:  [type=float]         [name=nothing or scale]                         [value=float]
    RETURN: List of {ref_link:Vector2}s
 */
STATIC mp_obj_t engine_utilities_generate_polygon_hexagon(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("EngineUtilities: Generating Polygon Hexagon");
    
    if(n_args == 0){
        return mp_obj_new_list(6, (mp_obj_t[]){ vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f),        mp_obj_new_float(1.0f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f),   mp_obj_new_float(0.5f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f),   mp_obj_new_float(-0.5f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f),        mp_obj_new_float(-1.0f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f),  mp_obj_new_float(-0.5f)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f),  mp_obj_new_float(0.5f)})});
    }else if(n_args == 1){
        float scale = mp_obj_get_float(args[0]);
        return mp_obj_new_list(6, (mp_obj_t[]){ vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f*scale),        mp_obj_new_float(1.0f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f*scale),   mp_obj_new_float(0.5f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f*scale),   mp_obj_new_float(-0.5f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f*scale),        mp_obj_new_float(-1.0f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f*scale),  mp_obj_new_float(-0.5f*scale)}),
                                                vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f*scale),  mp_obj_new_float(0.5f*scale)})});
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_utilities_generate_polygon_hexagon_obj, 0, 1, engine_utilities_generate_polygon_hexagon);


/*  --- doc ---
    NAME: engine_utilities
    DESC: Module for common functions and objects used across multiple areas of the engine
    ATTR: [type=function] [name={ref_link:generate_polygon_rectangle}]           [value=function]
    ATTR: [type=function] [name={ref_link:generate_polygon_hexagon}]             [value=function]
*/
STATIC const mp_rom_map_elem_t engine_utilities_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_utilities) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_generate_polygon_rectangle), (mp_obj_t)&engine_utilities_generate_polygon_rectangle_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_generate_polygon_hexagon), (mp_obj_t)&engine_utilities_generate_polygon_hexagon_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_utilities_globals, engine_utilities_globals_table);

const mp_obj_module_t engine_utilities_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_utilities_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_utilities, engine_utilities_user_cmodule);