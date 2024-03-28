#include "tunnel_node.h"

#include "tunnel_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "resources/engine_texture_resource.h"
#include "utility/engine_time.h"
#include "display/engine_display_common.h"


// Class required functions
STATIC void tunnel_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_PRINTF("TunnelNode");
}


void tunnel_node_class_draw(engine_node_base_t *node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("TunnelNode: Drawing");
    
    engine_tunnel_node_class_obj_t *tunnel = node_base->node;
    texture_resource_class_obj_t *texture = tunnel->texture_resource;
    engine_node_base_t *camera_node_base = camera_node;
    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);

    float animation = millis() / 1000.0f;

    int shift_x = (int)(texture->width * 1.0f * animation);
    int shift_y = (int)(texture->height * 0.25f * animation);

    int shift_look_x = SCREEN_WIDTH / 2 + (int)(SCREEN_WIDTH / 2 * camera_rotation->x.value);
    int shift_look_y = SCREEN_HEIGHT / 2 + (int)(SCREEN_HEIGHT / 2 * camera_rotation->z.value);

    for(uint8_t sy=0; sy<SCREEN_HEIGHT; sy++){
        for(uint8_t sx=0; sx<SCREEN_WIDTH; sx++){
            uint32_t calc_x = sx + shift_look_x;
            uint32_t calc_y = sy + shift_look_y;

            int distance = (int)(32.0f * texture->height / sqrtf((float)((calc_x - SCREEN_WIDTH) * (calc_x - SCREEN_WIDTH) + (calc_y - SCREEN_HEIGHT) * (calc_y - SCREEN_HEIGHT)))) % texture->height;


            // int angle = (unsigned int)(0.5 * texture->width * atan2f((float)(y - SCREEN_HEIGHT), (float)(x - SCREEN_WIDTH)) / PI);
            // uint16_t color = texture->data[angle * texture->width + distance];
            // engine_draw_pixel(color, sx, sy);
        }
    }
}

// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool tunnel_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_tunnel_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
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
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
            return true;
        break;
        case MP_QSTR_texture:
            destination[0] = self->texture_resource;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool tunnel_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_tunnel_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_texture:
            self->texture_resource = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t tunnel_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing TunnelNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = tunnel_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = tunnel_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }
}


/*  --- doc ---
    NAME: TunnelNode
    DESC: 
*/
mp_obj_t tunnel_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TunnelNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_texture,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, texture};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &texture_resource_class_type){
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

    if(parsed_args[texture].u_obj == MP_OBJ_NULL) parsed_args[texture].u_obj = mp_const_none;

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, NULL, &engine_tunnel_node_class_type, NODE_TYPE_TUNNEL);

    engine_tunnel_node_class_obj_t *tunnel_node = m_malloc(sizeof(engine_tunnel_node_class_obj_t));
    node_base->node = tunnel_node;
    node_base->attr_accessor = node_base;

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, tunnel_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t tunnel_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(tunnel_node_class_locals_dict, tunnel_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_tunnel_node_class_type,
    MP_QSTR_TunnelNode,
    MP_TYPE_FLAG_NONE,

    make_new, tunnel_node_class_new,
    print, tunnel_node_class_print,
    attr, tunnel_node_class_attr,
    locals_dict, &tunnel_node_class_locals_dict
);