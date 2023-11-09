#include "camera_node.h"

#include "node_types.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "engine_cameras.h"
#include "../display/engine_display_common.h"

// Class required functions
STATIC void camera_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): CameraNode");
}


STATIC mp_obj_t camera_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("CameraNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_tick_obj, camera_node_class_tick);


mp_obj_t camera_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New CameraNode");

    engine_camera_node_common_data_t *common_data = malloc(sizeof(engine_camera_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_camera_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_CAMERA;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    common_data->camera_list_node = engine_camera_track(node_base);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    mp_obj_t rectangle_args[4];
    rectangle_args[0] = mp_obj_new_float(0.0f);
    rectangle_args[1] = mp_obj_new_float(0.0f);
    rectangle_args[2] = mp_obj_new_float(SCREEN_WIDTH);
    rectangle_args[3] = mp_obj_new_float(SCREEN_HEIGHT);

    if(n_args == 0){        // Non-inherited (create a new object)
        engine_camera_node_class_obj_t *camera_node = m_malloc(sizeof(engine_camera_node_class_obj_t));
        node_base->node = camera_node;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&camera_node_class_tick_obj);

        camera_node->position = vector3_class_new(&vector3_class_type, 0, 0, NULL);
        camera_node->rotation = vector3_class_new(&vector3_class_type, 0, 0, NULL);
        camera_node->viewport = rectangle_class_new(&rectangle_class_type, 4, 0, rectangle_args);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->node = args[0];

        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&camera_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector3_class_new(&vector3_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_rotation, vector3_class_new(&vector3_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_viewport, rectangle_class_new(&rectangle_class_type, 4, 0, rectangle_args));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to CameraNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC mp_obj_t camera_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("CameraNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_camera_node_common_data_t *common_data = node_base->node_common_data;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    engine_camera_untrack(common_data->camera_list_node);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_del_obj, camera_node_class_del);


STATIC mp_obj_t camera_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(camera_node_class_set_layer_obj, camera_node_class_set_layer);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void camera_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CameraNode attr");

    engine_camera_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        if(attribute == MP_QSTR_position){
            destination[0] = self->position;
        }if(attribute == MP_QSTR_rotation){
            destination[0] = self->rotation;
        }else if(attribute == MP_QSTR_viewport){
            destination[0] = self->viewport;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(attribute == MP_QSTR_position){
            ENGINE_WARNING_PRINTF("Setting position not implemented!");
        }if(attribute == MP_QSTR_position){
            ENGINE_WARNING_PRINTF("Setting rotation not implemented!");
        }else if(attribute == MP_QSTR_viewport){
            ENGINE_WARNING_PRINTF("Setting viewport not implemented!");
        }else{
            return; // Fail
        }
        
        // Success
        destination[0] = MP_OBJ_NULL; 
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