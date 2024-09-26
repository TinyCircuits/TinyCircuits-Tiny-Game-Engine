#include "empty_node.h"

#include "node_types.h"
#include "node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool empty_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    engine_empty_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool empty_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_empty_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t empty_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing VoxelspaceNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){empty_node_load_attr, node_base_load_attr},
                          (attr_handler_func[]){empty_node_store_attr, node_base_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: EmptyNode
    ID: EmptyNode
    DESC: Node that does nothing except expose overrides for user implementation
    PARAM: [type={ref_link:Vector3}]     [name=position]                                    [value={ref_link:Vector3}]
    PARAM: [type={ref_link:Vector3}]     [name=rotation]                                    [value={ref_link:Vector3}]
    PARAM: [type=int]                    [name=layer]                                       [value=0 ~ 127]
    ATTR:  [type=function]               [name={ref_link:add_child}]                        [value=function] 
    ATTR:  [type=function]               [name={ref_link:get_child}]                        [value=function]
    ATTR:  [type=function]               [name={ref_link:get_child_count}]                  [value=function]
    ATTR:  [type=function]               [name={ref_link:node_base_mark_destroy}]           [value=function]
    ATTR:  [type=function]               [name={ref_link:node_base_mark_destroy_all}]       [value=function]
    ATTR:  [type=function]               [name={ref_link:node_base_mark_destroy_children}]  [value=function]
    ATTR:  [type=function]               [name={ref_link:remove_child}]                     [value=function]
    ATTR:  [type=function]               [name={ref_link:get_parent}]                       [value=function]
    ATTR:  [type=function]               [name={ref_link:tick}]                             [value=function]
    ATTR:  [type={ref_link:Vector3}]     [name=position]                                    [value={ref_link:Vector3}]
    ATTR:  [type={ref_link:Vector3}]     [name=rotation]                                    [value={ref_link:Vector3}]
    ATTR:  [type=int]                    [name=layer]                                       [value=0 ~ 127]
    OVRR:  [type=function]               [name={ref_link:tick}]                             [value=function]
*/
static mp_obj_t empty_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New EmptyNode");

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)})} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)})} },
        { MP_QSTR_layer,        MP_ARG_INT, {.u_int = 0} }
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, rotation, layer};
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

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_empty_node_class_type);
    node_base_init(node_base, &engine_empty_node_class_type, NODE_TYPE_EMPTY, parsed_args[layer].u_int);
    engine_empty_node_class_obj_t *empty_node = m_malloc(sizeof(engine_empty_node_class_obj_t));
    node_base->node = empty_node;
    node_base->attr_accessor = node_base;

    empty_node->position = parsed_args[position].u_obj;
    empty_node->rotation = parsed_args[rotation].u_obj;
    empty_node->tick_cb = mp_const_none;

    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        node_base_set_attr_handler_default(node_instance);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];

        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            empty_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            empty_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, empty_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t empty_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(empty_node_class_locals_dict, empty_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_empty_node_class_type,
    MP_QSTR_EmptyNode,
    MP_TYPE_FLAG_NONE,

    make_new, empty_node_class_new,
    attr, empty_node_class_attr,
    locals_dict, &empty_node_class_locals_dict
);