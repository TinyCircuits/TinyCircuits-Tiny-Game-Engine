#include "camera_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"
#include "display/engine_display_common.h"
#include "engine_cameras.h"
#include "math/engine_math.h"


// https://stackoverflow.com/a/54958473
const vector3_class_obj_t world_up = {
    .base.type = &vector3_class_type,

    .x = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = 0.0f,
    },
    .y = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = 1.0f,
    },
    .z = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = 0.0f,
    },
};


const vector3_class_obj_t world_north = {
    .base.type = &vector3_class_type,

    .x = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = 0.0f,
    },
    .y = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = -1.0f,
    },
    .z = (mp_obj_float_t){.base.type = &mp_type_float,
                          .value = 0.0f,
    },
};


// Class required functions
STATIC void camera_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): CameraNode");
}


// // https://forums.unrealengine.com/t/how-does-get-look-at-rotation-work-from-a-mathematical-point-of-view/732711/3
// // https://gamedev.stackexchange.com/a/112572
// STATIC mp_obj_t camera_node_class_lookat(mp_obj_t self_in, mp_obj_t lookat_target_position_obj){
//     ENGINE_WARNING_PRINTF("CameraNode: Lookat");

//     engine_node_base_t *camera_node_base = self_in;

//     vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
//     vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
//     vector3_class_obj_t *lookat_target_position = lookat_target_position_obj;

//     float lookat_x = lookat_target_position->x.value - camera_position->x.value;
//     float lookat_y = lookat_target_position->y.value - camera_position->y.value;
//     float lookat_z = lookat_target_position->z.value - camera_position->z.value;
//     engine_math_3d_normalize(&lookat_x, &lookat_y, &lookat_z);

//     camera_rotation->x.value = asinf(lookat_y);
//     camera_rotation->y.value = atan2f(lookat_x, lookat_z);
//     // camera_rotation->y.value = look_up_y;
//     // camera_rotation->z.value = look_up_z;

//     // // Step 1: generate x
//     // float forward_x = lookat_target_position->x.value - camera_position->x.value;
//     // float forward_y = lookat_target_position->y.value - camera_position->y.value;
//     // float forward_z = lookat_target_position->z.value - camera_position->z.value;

//     // engine_math_3d_normalize(&forward_x, &forward_y, &forward_z);

//     // // Step 2: generate y
//     // float world_up_x = 0.0f;
//     // float world_up_y = 1.0f;
//     // float world_up_z = 0.0f;
//     // float side_x = 0.0f;
//     // float side_y = 0.0f;
//     // float side_z = 0.0f;

//     // engine_math_3d_cross_product_v_v(world_up_x, world_up_y, world_up_z,
//     //                                  forward_x, forward_y, forward_z,
//     //                                  &side_x, &side_y, &side_z);
    
//     // // Step 3: generate z
//     // float look_up_x = 0.0f;
//     // float look_up_y = 0.0f;
//     // float look_up_z = 0.0f;
//     // engine_math_3d_cross_product_v_v(forward_x, forward_y, forward_z,
//     //                                  side_x, side_y, side_z,
//     //                                  &look_up_x, &look_up_y, &look_up_z);
    
//     // camera_rotation->x.value = look_up_x;
//     // camera_rotation->y.value = look_up_y;
//     // camera_rotation->z.value = look_up_z;

//     return mp_const_none;
// }
// MP_DEFINE_CONST_FUN_OBJ_2(camera_node_class_lookat_obj, camera_node_class_lookat);


mp_obj_t camera_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("CameraNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_camera_node_class_obj_t *camera_node = node_base->node;
    engine_camera_untrack(camera_node->camera_list_node);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_del_obj, camera_node_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool camera_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_camera_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&camera_node_class_del_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_add_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_remove_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_set_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        // case MP_QSTR_lookat:
        //     destination[0] = MP_OBJ_FROM_PTR(&camera_node_class_lookat_obj);
        //     destination[1] = self_node_base;
        //     return true;
        // break;
        case MP_QSTR_tick:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_zoom:
            destination[0] = self->zoom;
            return true;
        break;
        case MP_QSTR_viewport:
            destination[0] = self->viewport;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_fov:
            destination[0] = self->fov;
            return true;
        break;
        case MP_QSTR_view_distance:
            destination[0] = self->view_distance;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool camera_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_camera_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_zoom:
            self->zoom = destination[1];
            return true;
        break;
        case MP_QSTR_viewport:
            self->viewport = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_fov:
            self->fov = destination[1];
            return true;
        break;
        case MP_QSTR_view_distance:
            self->view_distance = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t camera_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CameraNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = camera_node_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = camera_node_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: CameraNode
    DESC: Node that defines the perspective the scene is drawn at. There can be multiple but this will impact performance if rendering the same scene twice. To make other nodes not move when the camera moves, make the other nodes children of the camera.
    PARAM: [type={ref_link:Vector3}]             [name=position]                 [value={ref_link:Vector3}]
    PARAM: [type=float]                          [name=zoom]                     [value=any (scales all nodes by this factor, 1.0 by default)]
    PARAM: [type={ref_link:Rectangle}]           [name=viewport]                 [value={ref_link:Rectangle} (not used currently, TODO)]
    PARAM: [type={ref_link:Vector3}]             [name=rotation]                 [value={ref_link:Vector3}]
    PARAM: [type=float]                          [name=fov]                      [value=any (sets the field fo view for rendering some nodes, not all nodes use this)]
    PARAM: [type=float]                          [name=view_distance]            [value=any (sets the view distance for some nodes, not all nodes use this)]
    ATTR:  [type=function]                       [name={ref_link:add_child}]     [value=function] 
    ATTR:  [type=function]                       [name={ref_link:get_child}]     [value=function] 
    ATTR:  [type=function]                       [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type=function]                       [name={ref_link:set_layer}]     [value=function]
    ATTR:  [type=function]                       [name={ref_link:get_layer}]     [value=function]
    ATTR:  [type=function]                       [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type=function]                       [name={ref_link:tick}]          [value=function]
    ATTR:  [type={ref_link:Vector3}]             [name=position]                 [value={ref_link:Vector3}]
    ATTR:  [type={ref_link:Vector3}]             [name=rotation]                 [value={ref_link:Vector3}]
    ATTR:  [type=float]                          [name=zoom]                     [value=any (scales all nodes by this factor, 1.0 by default)]
    ATTR:  [type={ref_link:Rectangle}]           [name=viewport]                 [value={ref_link:Rectangle} (not used currently, TODO)]
    ATTR:  [type=float]                          [name=fov]                      [value=any (sets the field fo view for rendering some nodes, not all nodes use this)]
    ATTR:  [type=float]                          [name=view_distance]            [value=any (sets the view distance for some nodes, not all nodes use this)]
    OVRR:  [type=function]                       [name={ref_link:tick}]          [value=function]
*/
mp_obj_t camera_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Sprite2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_zoom,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_viewport,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fov,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_view_distance,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, rotation, zoom, viewport, fov, view_distance};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector3_class_type){
        // Using positional arguments but the type of the first one isn't
        // as expected. Must be the child class
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);
        inherited = true;
    }else{
        // Whether we're using positional arguments or not, prase them this
        // way. It's a requirement that the child class be passed using position.
        // Adjust what and where the arguments are parsed, since not inherited based
        // on the first argument
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args)-1, allowed_args+1, parsed_args+1);
        inherited = false;
    }

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL);
    if(parsed_args[zoom].u_obj == MP_OBJ_NULL) parsed_args[zoom].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[viewport].u_obj == MP_OBJ_NULL) parsed_args[viewport].u_obj = rectangle_class_new(&rectangle_class_type, 4, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float((float)SCREEN_WIDTH), mp_obj_new_float((float)SCREEN_HEIGHT)});
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL);
    if(parsed_args[fov].u_obj == MP_OBJ_NULL) parsed_args[fov].u_obj = mp_obj_new_float(PI/2.0f);
    if(parsed_args[view_distance].u_obj == MP_OBJ_NULL) parsed_args[view_distance].u_obj = mp_obj_new_float(256.0f);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, NULL, &engine_camera_node_class_type, NODE_TYPE_CAMERA);
    engine_camera_node_class_obj_t *camera_node = m_malloc(sizeof(engine_camera_node_class_obj_t));
    node_base->node = camera_node;
    node_base->attr_accessor = node_base;

    // Track the node base for this camera so that it can be
    // passed to draw callbacks, determined if inherited or not,
    // and then atributes looked up and used for drawing
    camera_node->camera_list_node = engine_camera_track(node_base);
    camera_node->tick_cb = mp_const_none;
    camera_node->position = parsed_args[position].u_obj;
    camera_node->zoom = parsed_args[zoom].u_obj;
    camera_node->viewport = parsed_args[viewport].u_obj;
    camera_node->rotation = parsed_args[rotation].u_obj;
    camera_node->fov = parsed_args[fov].u_obj;
    camera_node->view_distance = parsed_args[view_distance].u_obj;
    
    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];

        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            camera_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            camera_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, camera_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t camera_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(camera_node_class_locals_dict, camera_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_camera_node_class_type,
    MP_QSTR_CameraNode,
    MP_TYPE_FLAG_NONE,

    make_new, camera_node_class_new,
    print, camera_node_class_print,
    attr, camera_node_class_attr,
    locals_dict, &camera_node_class_locals_dict
);