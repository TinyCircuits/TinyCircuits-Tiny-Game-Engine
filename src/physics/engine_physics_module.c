#include "py/obj.h"
#include "engine_physics_module.h"
#include "nodes/node_base.h"
#include "display/engine_display_common.h"
#include "physics/engine_physics.h"
#include "physics/collision_shapes/polygon_collision_shape_2d.h"
#include "physics/collision_contact_2d.h"


STATIC mp_obj_t engine_set_physics_fps_limit(mp_obj_t fps){
    ENGINE_INFO_PRINTF("EnginePhysics: Setting FPS Limit");
    engine_physics_fps_limit_period_ms = (1.0f / mp_obj_get_float(fps)) * 1000.0f;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_physics_fps_limit_obj, engine_set_physics_fps_limit);


// Could add another method that takes a vector2 to set gravity, just this for now
STATIC mp_obj_t engine_set_physics_set_gravity(mp_obj_t x, mp_obj_t y){
    ENGINE_INFO_PRINTF("EnginePhysics: Setting gravity");
    engine_physics_gravity_x = mp_obj_get_float(x);
    engine_physics_gravity_y = mp_obj_get_float(y);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_set_physics_set_gravity_obj, engine_set_physics_set_gravity);


STATIC mp_obj_t engine_set_physics_get_gravity(){
    ENGINE_INFO_PRINTF("EnginePhysics: Getting gravity");
    return vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(engine_physics_gravity_x), mp_obj_new_float(engine_physics_gravity_y)});
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_set_physics_get_gravity_obj, engine_set_physics_get_gravity);


// Module attributes
STATIC const mp_rom_map_elem_t engine_physics_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_physics) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_EmptyPolyCollisionShape2D), (mp_obj_t)&empty_poly_collision_shape_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_HexagonPolyCollisionShape2D), (mp_obj_t)&hexagon_poly_collision_shape_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_RectanglePolyCollisionShape2D), (mp_obj_t)&rectangle_poly_collision_shape_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CollisionContact2D), (mp_obj_t)&collision_contact_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_physics_fps_limit), (mp_obj_t)&engine_set_physics_fps_limit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_gravity), (mp_obj_t)&engine_set_physics_set_gravity_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_gravity), (mp_obj_t)&engine_set_physics_get_gravity_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_physics_globals, engine_physics_globals_table);

const mp_obj_module_t engine_physics_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_physics_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_physics, engine_physics_user_cmodule);
