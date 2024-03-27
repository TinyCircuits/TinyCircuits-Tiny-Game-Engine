#include "camera_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"
#include "display/engine_display_common.h"
#include "engine_cameras.h"
#include "math/engine_math.h"


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
    
    engine_camera_node_common_data_t *common_data = malloc(sizeof(engine_camera_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_camera_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_CAMERA;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    // Track the node base for this camera so that it can be
    // passed to draw callbacks, determined if inherited or not,
    // and then atributes looked up and used for drawing
    common_data->camera_list_node = engine_camera_track(node_base);

    if(inherited == false){        // Non-inherited (create a new object)
        engine_camera_node_class_obj_t *camera_node = m_malloc(sizeof(engine_camera_node_class_obj_t));
        node_base->node = camera_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&camera_node_class_tick_obj);

        camera_node->position = parsed_args[position].u_obj;
        camera_node->zoom = parsed_args[zoom].u_obj;
        camera_node->viewport = parsed_args[viewport].u_obj;
        camera_node->rotation = parsed_args[rotation].u_obj;
        camera_node->fov = parsed_args[fov].u_obj;
        camera_node->view_distance = parsed_args[view_distance].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->node = parsed_args[child_class].u_obj;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&camera_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_zoom, parsed_args[zoom].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_viewport, parsed_args[viewport].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_fov, parsed_args[fov].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_view_distance, parsed_args[view_distance].u_obj);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


mp_obj_t camera_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("CameraNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_camera_node_common_data_t *common_data = node_base->node_common_data;
    engine_camera_untrack(common_data->camera_list_node);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(camera_node_class_del_obj, camera_node_class_del);


STATIC void camera_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CameraNode attr");

    engine_camera_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&camera_node_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_add_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_remove_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_node_base:
                destination[0] = self_in;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_zoom:
                destination[0] = self->zoom;
            break;
            case MP_QSTR_viewport:
                destination[0] = self->viewport;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_fov:
                destination[0] = self->fov;
            break;
            case MP_QSTR_view_distance:
                destination[0] = self->view_distance;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_zoom:
                self->zoom = destination[1];
            break;
            case MP_QSTR_viewport:
                self->viewport = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_fov:
                self->fov = destination[1];
            break;
            case MP_QSTR_view_distance:
                self->view_distance = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
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