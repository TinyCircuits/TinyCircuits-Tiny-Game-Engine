#include "physics_circle_2d_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "nodes/physics_node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_display_draw.h"
#include "physics/engine_physics.h"
#include "physics/engine_physics_ids.h"


mp_obj_t physics_circle_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("PhysicsCircle2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_physics_node_base_t *physics_node_base = node_base->node;
    engine_physics_circle_2d_node_class_obj_t *node = physics_node_base->unique_data;
    engine_physics_untrack_node(physics_node_base->physics_list_node);
    engine_physics_ids_give_back(physics_node_base->physics_id);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(physics_circle_2d_node_class_del_obj, physics_circle_2d_node_class_del);


void physics_circle_2d_calculate_inverse_mass(engine_node_base_t *node_base){
    engine_physics_node_base_t *physics_node_base = node_base->node;
    engine_physics_circle_2d_node_class_obj_t *physics_circle = physics_node_base->unique_data;

    float radius = mp_obj_get_float(physics_circle->radius);
    float density = mp_obj_get_float(physics_node_base->density);
    float area = PI * radius*radius;
    physics_node_base->mass = density * area;

    // https://www.concepts-of-physics.com/mechanics/moment-of-inertia.php#:~:text=Moment%20of%20Inertia%20of%20Common%20Shapes
    if(density == 0.0f){
        physics_node_base->inverse_mass = 0.0f;
    }else{
        physics_node_base->inverse_mass = 1.0f / physics_node_base->mass;
    }
}


void physics_circle_2d_calculate_inverse_inertia(engine_node_base_t *node_base){
    engine_physics_node_base_t *physics_node_base = node_base->node;
    engine_physics_circle_2d_node_class_obj_t *physics_circle = physics_node_base->unique_data;

    float radius = mp_obj_get_float(physics_circle->radius);
    float mass = physics_node_base->mass;

    // https://www.concepts-of-physics.com/mechanics/moment-of-inertia.php#:~:text=Moment%20of%20Inertia%20of%20Common%20Shapes
    if(mass == 0.0f){
        physics_node_base->inverse_moment_of_inertia = 0.0f;
    }else{
        physics_node_base->inverse_moment_of_inertia = 1.0f / (0.5f + mass * (radius*radius));
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_circle_2d_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *physics_node_base = self_node_base->node;
    engine_physics_circle_2d_node_class_obj_t *self = physics_node_base->unique_data;

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
        case MP_QSTR_radius:
            destination[0] = self->radius;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool physics_circle_2d_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_physics_node_base_t *physics_node_base = self_node_base->node;
    engine_physics_circle_2d_node_class_obj_t *self = physics_node_base->unique_data;

    switch(attribute){
        case MP_QSTR_radius:
            self->radius = destination[1];

            // As the radius changes so does the mass due to density
            physics_circle_2d_calculate_inverse_mass(self_node_base);
            physics_circle_2d_calculate_inverse_inertia(self_node_base);
            return true;
        break;
        case MP_QSTR_density:
            physics_node_base->density = destination[1];
            physics_circle_2d_calculate_inverse_mass(self_node_base);
            physics_circle_2d_calculate_inverse_inertia(self_node_base);
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
        if(!attr_handled) attr_handled = physics_node_base_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = physics_circle_2d_store_attr(node_base, attribute, destination);
        if(!attr_handled) attr_handled = physics_node_base_store_attr(node_base, attribute, destination);

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
    NAME: PhysicsCircle2DNode
    ID: PhysicsCircle2DNode
    DESC: Node that is affected by physics. Usually other nodes are added as children to this node
    PARAM: [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    PARAM: [type=float]                                  [name=radius]                   [value=any]
    PARAM: [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    PARAM: [type=float]                                  [name=rotation]                 [value=any]
    PARAM: [type=float]                                  [name=density]                  [value=any]
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
    ATTR:  [type=function]                               [name={ref_link:tick}]          [value=function]
    ATTR:  [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    ATTR:  [type=float]                                  [name=radius]                   [value=any]
    ATTR:  [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    ATTR:  [type=float]                                  [name=rotation]                 [value=any]
    ATTR:  [type=float]                                  [name=density]                  [value=any]
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
        { MP_QSTR_angular_velocity, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_density,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_friction,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bounciness,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dynamic,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_solid,            MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_gravity_scale,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, radius, velocity, angular_velocity, rotation, density, friction, bounciness, dynamic, solid, gravity_scale};
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
    if(parsed_args[angular_velocity].u_obj == MP_OBJ_NULL) parsed_args[angular_velocity].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0);
    if(parsed_args[density].u_obj == MP_OBJ_NULL) parsed_args[density].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[friction].u_obj == MP_OBJ_NULL) parsed_args[friction].u_obj = mp_obj_new_float(0.1f);
    if(parsed_args[bounciness].u_obj == MP_OBJ_NULL) parsed_args[bounciness].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[dynamic].u_obj == MP_OBJ_NULL) parsed_args[dynamic].u_obj = mp_obj_new_int(1);
    if(parsed_args[solid].u_obj == MP_OBJ_NULL) parsed_args[solid].u_obj = mp_obj_new_int(1);
    if(parsed_args[gravity_scale].u_obj == MP_OBJ_NULL) parsed_args[gravity_scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, &engine_physics_circle_2d_node_class_type, NODE_TYPE_PHYSICS_CIRCLE_2D);

    // Another layer, all physics objects have some data in common,
    // create that plus the specific data structure for this collider
    engine_physics_node_base_t *physics_node_base = m_malloc(sizeof(engine_physics_node_base_t));
    engine_physics_circle_2d_node_class_obj_t *physics_circle_2d_node = m_malloc(sizeof(engine_physics_circle_2d_node_class_obj_t));
    physics_node_base->unique_data = physics_circle_2d_node;

    node_base->node = physics_node_base;
    node_base->attr_accessor = node_base;

    physics_node_base->position = parsed_args[position].u_obj;
    physics_node_base->velocity = parsed_args[velocity].u_obj;
    physics_node_base->angular_velocity = mp_obj_get_float(parsed_args[angular_velocity].u_obj);
    physics_node_base->rotation = mp_obj_get_float(parsed_args[rotation].u_obj);
    physics_node_base->density = parsed_args[density].u_obj;
    physics_node_base->friction = parsed_args[friction].u_obj;
    physics_node_base->bounciness = parsed_args[bounciness].u_obj;
    physics_node_base->dynamic = parsed_args[dynamic].u_obj;
    physics_node_base->solid = parsed_args[solid].u_obj;
    physics_node_base->gravity_scale = parsed_args[gravity_scale].u_obj;
    physics_node_base->physics_id = engine_physics_ids_take_available();
    physics_node_base->mass = 0.0f;
    physics_node_base->total_position_correction_x = 0.0f;
    physics_node_base->total_position_correction_y = 0.0f;

    // Track the node base for this physics node so that it can
    // be looped over quickly in a linked list
    physics_node_base->physics_list_node = engine_physics_track_node(node_base);

    physics_node_base->tick_cb = mp_const_none;
    physics_node_base->collision_cb = mp_const_none;

    physics_circle_2d_node->radius = parsed_args[radius].u_obj;

    physics_circle_2d_calculate_inverse_mass(node_base);
    physics_circle_2d_calculate_inverse_inertia(node_base);

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];

        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            physics_node_base->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            physics_node_base->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_collision, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            physics_node_base->collision_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            physics_node_base->collision_cb = dest[0];
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
    attr, physics_circle_2d_node_class_attr,
    locals_dict, &physics_circle_2d_node_class_locals_dict
);