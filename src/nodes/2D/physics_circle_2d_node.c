#include "physics_circle_2d_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_display_draw.h"


// Class required functions
STATIC void physics_circle_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): PhysicsCircle2DNode");
}


STATIC mp_obj_t physics_circle_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("PhysicsCircle2DNode: Tick callback not overridden!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_circle_2d_node_class_tick_obj, physics_circle_2d_node_class_tick);


STATIC mp_obj_t physics_circle_2d_node_class_collision(mp_obj_t self_in, mp_obj_t collision_contact_2d){
    ENGINE_WARNING_PRINTF("PhysicsCircle2DNode: Collision callback not overridden!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_circle_2d_node_class_collision_obj, physics_circle_2d_node_class_collision);


mp_obj_t physics_circle_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("PhysicsCircle2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_physics_circle_2d_node_class_obj_t *node = node_base->node;
    engine_physics_untrack_node(node->physics_list_node);
    engine_physics_give_back_id(node->physics_id);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(physics_circle_2d_node_class_del_obj, physics_circle_2d_node_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_circle_2d_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_circle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&physics_circle_2d_node_class_del_obj);
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
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_radius:
            destination[0] = self->radius;
            return true;
        break;
        case MP_QSTR_velocity:
            destination[0] = self->velocity;
            return true;
        break;
        case MP_QSTR_acceleration:
            destination[0] = self->acceleration;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_mass:
            destination[0] = self->mass;
            return true;
        break;
        case MP_QSTR_bounciness:
            destination[0] = self->bounciness;
            return true;
        break;
        case MP_QSTR_dynamic:
            destination[0] = self->dynamic;
            return true;
        break;
        case MP_QSTR_solid:
            destination[0] = self->solid;
            return true;
        break;
        case MP_QSTR_gravity_scale:
            destination[0] = self->gravity_scale;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool physics_circle_2d_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_circle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_radius:
            self->radius = destination[1];
            return true;
        break;
        case MP_QSTR_velocity:
            self->velocity = destination[1];
            return true;
        break;
        case MP_QSTR_acceleration:
            self->acceleration = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_mass:
            self->mass = destination[1];
            return true;
        break;
        case MP_QSTR_bounciness:
            self->bounciness = destination[1];
            return true;
        break;
        case MP_QSTR_dynamic:
            self->dynamic = destination[1];
            return true;
        break;
        case MP_QSTR_solid:
            self->solid = destination[1];
            return true;
        break;
        case MP_QSTR_gravity_scale:
            self->gravity_scale = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t physics_circle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsCircle2DNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = physics_circle_2d_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = physics_circle_2d_store_attr(node_base, attribute, destination);

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
    NAME: PhysicsCircle2DNode
    DESC: Node that is affected by physics. Usually other nodes are added as children to this node
    PARAM: [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    PARAM: [type=float]                                  [name=radius]                   [value=any]
    PARAM: [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    PARAM: [type={ref_link:Vector2}]                     [name=acceleration]             [value={ref_link:Vector2}]
    PARAM: [type=float]                                  [name=rotation]                 [value=any]
    PARAM: [type=float]                                  [name=mass]                     [value=any]
    PARAM: [type=float]                                  [name=bounciness]               [value=any]
    PARAM: [type=boolean]                                [name=dynamic]                  [value=True or False]
    PARAM: [type=boolean]                                [name=solid]                    [value=True or False]
    PARAM: [type={ref_link:Vector2}]                     [name=gravity_scale]            [value={ref_link:Vector2}]
    ATTR:  [type=function]                               [name={ref_link:add_child}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:get_child}]     [value=function] 
    ATTR:  [type=function]                               [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type=function]                               [name={ref_link:set_layer}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:get_layer}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    ATTR:  [type=float]                                  [name=radius]                   [value=any]
    ATTR:  [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    ATTR:  [type={ref_link:Vector2}]                     [name=acceleration]             [value={ref_link:Vector2}]
    ATTR:  [type=float]                                  [name=rotation]                 [value=any]
    ATTR:  [type=float]                                  [name=mass]                     [value=any]
    ATTR:  [type=float]                                  [name=bounciness]               [value=any]
    ATTR:  [type=boolean]                                [name=dynamic]                  [value=True or False]
    ATTR:  [type=boolean]                                [name=solid]                    [value=True or False]
    ATTR:  [type={ref_link:Vector2}]                     [name=gravity_scale]            [value={ref_link:Vector2}]
    OVRR:  [type=function]                               [name={ref_link:tick}]          [value=function]
*/
mp_obj_t physics_circle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PhysicsCircle2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_radius,           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_velocity,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_acceleration,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mass,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bounciness,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dynamic,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_solid,            MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_gravity_scale,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, radius, velocity, acceleration, rotation, mass, bounciness, dynamic, solid, gravity_scale};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector2_class_type){
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

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[radius].u_obj == MP_OBJ_NULL) parsed_args[radius].u_obj = mp_obj_new_float(5.0f);
    if(parsed_args[velocity].u_obj == MP_OBJ_NULL) parsed_args[velocity].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[acceleration].u_obj == MP_OBJ_NULL) parsed_args[acceleration].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0);
    if(parsed_args[mass].u_obj == MP_OBJ_NULL) parsed_args[mass].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[bounciness].u_obj == MP_OBJ_NULL) parsed_args[bounciness].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[dynamic].u_obj == MP_OBJ_NULL) parsed_args[dynamic].u_obj = mp_obj_new_int(1);
    if(parsed_args[solid].u_obj == MP_OBJ_NULL) parsed_args[solid].u_obj = mp_obj_new_int(1);
    if(parsed_args[gravity_scale].u_obj == MP_OBJ_NULL) parsed_args[gravity_scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, NULL, &engine_physics_circle_2d_node_class_type, NODE_TYPE_PHYSICS_CIRCLE_2D);

    engine_physics_circle_2d_node_class_obj_t *physics_2d_node = m_malloc(sizeof(engine_physics_circle_2d_node_class_obj_t));
    node_base->node = physics_2d_node;
    node_base->attr_accessor = node_base;

    physics_2d_node->position = parsed_args[position].u_obj;
    physics_2d_node->radius = parsed_args[radius].u_obj;
    physics_2d_node->velocity = parsed_args[velocity].u_obj;
    physics_2d_node->acceleration = parsed_args[acceleration].u_obj;
    physics_2d_node->rotation = parsed_args[rotation].u_obj;
    physics_2d_node->mass = parsed_args[mass].u_obj;
    physics_2d_node->bounciness = parsed_args[bounciness].u_obj;
    physics_2d_node->dynamic = parsed_args[dynamic].u_obj;
    physics_2d_node->solid = parsed_args[solid].u_obj;
    physics_2d_node->gravity_scale = parsed_args[gravity_scale].u_obj;

    // Track the node base for this physics node so that it can
    // be looped over quickly in a linked list
    physics_2d_node->physics_list_node = engine_physics_track_node(node_base);

    physics_2d_node->tick_cb = MP_OBJ_FROM_PTR(&physics_circle_2d_node_class_tick_obj);
    physics_2d_node->collision_cb = MP_OBJ_FROM_PTR(&physics_circle_2d_node_class_collision_obj);

    physics_2d_node->physics_id = engine_physics_take_available_id();


    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            physics_2d_node->tick_cb = MP_OBJ_FROM_PTR(&physics_circle_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            physics_2d_node->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_collision, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            physics_2d_node->collision_cb = MP_OBJ_FROM_PTR(&physics_circle_2d_node_class_collision_obj);
        }else{                                                  // Likely found method (could be attribute)
            physics_2d_node->collision_cb = dest[0];
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
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, physics_circle_2d_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t physics_circle_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(physics_circle_2d_node_class_locals_dict, physics_circle_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_physics_circle_2d_node_class_type,
    MP_QSTR_PhysicsCircle2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, physics_circle_2d_node_class_new,
    print, physics_circle_2d_node_class_print,
    attr, physics_circle_2d_node_class_attr,
    locals_dict, &physics_circle_2d_node_class_locals_dict
);