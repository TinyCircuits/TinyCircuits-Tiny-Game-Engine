#include "py/obj.h"
#include "engine_physics_module.h"
#include "nodes/node_base.h"
#include "display/engine_display_common.h"
#include "physics/engine_physics.h"
#include "physics/collision_shapes/circle_collision_shape_2d.h"
#include "physics/collision_contact_2d.h"


STATIC mp_obj_t engine_set_physics_fps(mp_obj_t fps){
    ENGINE_INFO_PRINTF("EnginePhysics: Setting FPS");
    engine_physics_fps = mp_obj_get_float(fps);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_physics_fps_obj, engine_set_physics_fps);


// Module attributes
STATIC const mp_rom_map_elem_t engine_physics_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_physics) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CircleCollisionShape2D), (mp_obj_t)&circle_collision_shape_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_CollisionContact2D), (mp_obj_t)&collision_contact_2d_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_physics_fps), (mp_obj_t)&engine_set_physics_fps_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_physics_globals, engine_physics_globals_table);

const mp_obj_module_t engine_physics_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_physics_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_physics, engine_physics_user_cmodule);
