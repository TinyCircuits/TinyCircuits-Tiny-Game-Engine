#include "py/obj.h"
#include "engine_physics_module.h"
#include "nodes/node_base.h"
#include "display/engine_display_common.h"
#include "physics/engine_physics.h"
#include "physics/shapes/engine_physics_shape_rectangle.h"
#include "physics/shapes/engine_physics_shape_circle.h"
#include "physics/shapes/engine_physics_shape_convex.h"
#include "physics/shapes/engine_physics_manifold.h"


// Module attributes
STATIC const mp_rom_map_elem_t engine_physics_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_physics) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PhysicsShapeRectangle), (mp_obj_t)&physics_shape_rectangle_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PhysicsShapeCircle), (mp_obj_t)&physics_shape_circle_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PhysicsShapeConvex), (mp_obj_t)&physics_shape_convex_class_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_PhysicsManifold), (mp_obj_t)&physics_manifold_class_type},
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_physics_globals, engine_physics_globals_table);

const mp_obj_module_t engine_physics_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_physics_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_physics, engine_physics_user_cmodule);
