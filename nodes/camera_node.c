#include "camera_node.h"

#include "node_types.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"
#include "math/vector3.h"
#include "engine_cameras.h"

// Class required functions
STATIC void camera_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): CameraNode");
}

mp_obj_t camera_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New CameraNode");
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_camera_node_class_obj_t *self = m_new_obj_with_finaliser(engine_camera_node_class_obj_t);
    self->base.type = &engine_camera_node_class_type;
    self->node_base.layer = 0;
    self->node_base.type = NODE_TYPE_CAMERA;
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);
    self->camera_list_node = engine_camera_track(self);
    node_base_set_if_visible(&self->node_base, true);
    node_base_set_if_disabled(&self->node_base, false);
    node_base_set_if_just_added(&self->node_base, true);

    // Cache lookup results of 'tick()' function on this node
    // instance so that the main engine loop can call it quickly
    mp_load_method(MP_OBJ_TO_PTR(args[0]), MP_QSTR_tick, self->tick_dest);

    ENGINE_INFO_PRINTF("Creating new Vector3 for CameraNode");
    self->position = vector3_class_new(&vector3_class_type, 0, 0, NULL);

    ENGINE_INFO_PRINTF("Creating new Rectangle for CameraNode viewport");
    self->viewport = rectangle_class_new(&rectangle_class_type, 0, 0, NULL);

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t camera_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("CameraNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_camera_node_class_obj_t *self = ((engine_camera_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);
    engine_camera_untrack(self->camera_list_node);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_del_obj, camera_node_class_del);


STATIC mp_obj_t camera_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("CameraNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_tick_obj, camera_node_class_tick);


STATIC mp_obj_t camera_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_camera_node_class_obj_t *self = ((engine_camera_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);
    self->node_base.layer = (uint16_t)mp_obj_get_int(layer);
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(camera_node_class_set_layer_obj, camera_node_class_set_layer);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void camera_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    engine_camera_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        if(attribute == MP_QSTR_position){
            destination[0] = self->position;
        }else if(attribute == MP_QSTR_viewport){
            destination[0] = self->viewport;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(attribute == MP_QSTR_position){
            // if(mp_obj_str_get_qstr(destination[1]))
            ENGINE_WARNING_PRINTF("Setting position not implemented!");
            destination[0] = MP_OBJ_NULL;   
        }else if(attribute == MP_QSTR_viewport){
            // if(mp_obj_str_get_qstr(destination[1]))
            ENGINE_WARNING_PRINTF("Setting viewport not implemented!");
            destination[0] = MP_OBJ_NULL; 
        }
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t camera_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),     MP_ROM_PTR(&camera_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick),        MP_ROM_PTR(&camera_node_class_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_layer),   MP_ROM_PTR(&camera_node_class_set_layer_obj) },
};


// Class init
STATIC MP_DEFINE_CONST_DICT(camera_node_class_locals_dict, camera_node_class_locals_dict_table);

const mp_obj_type_t engine_camera_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_CameraNode,
    .print = camera_node_class_print,
    .make_new = camera_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = camera_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&camera_node_class_locals_dict,
};