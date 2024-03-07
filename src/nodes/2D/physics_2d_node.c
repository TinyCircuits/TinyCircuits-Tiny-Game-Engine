#include "physics_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "draw/engine_display_draw.h"
#include "physics/engine_physics.h"


// Class required functions
STATIC void physics_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Physics2DNode");
}


STATIC mp_obj_t physics_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Physics2DNode: Tick callback not overridden!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_2d_node_class_tick_obj, physics_2d_node_class_tick);


STATIC mp_obj_t physics_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_WARNING_PRINTF("Physics2DNode: Drawing callback not overridden!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_2d_node_class_draw_obj, physics_2d_node_class_draw);


STATIC mp_obj_t physics_2d_node_class_collision(mp_obj_t self_in, mp_obj_t collision_contact_2d){
    ENGINE_WARNING_PRINTF("Physics2DNode: Collision callback not overridden!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_2d_node_class_collision_obj, physics_2d_node_class_collision);


/*  --- doc ---
    NAME: Physics2DNode
    DESC: Node that is affected by physics. Usually other nodes are added as children to this node
    PARAM: [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    PARAM: [type={ref_link:PolygonCollisionShape2D}]     [name=collision_shape]          [value={ref_link:PolygonCollisionShape2D}]
    PARAM: [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    PARAM: [type={ref_link:Vector2}]                     [name=acceleration]             [value={ref_link:Vector2}]
    PARAM: [type=float]                                  [name=rotation]                 [value=any]
    PARAM: [type=float]                                  [name=mass]                     [value=any]
    PARAM: [type=float]                                  [name=bounciness]               [value=any]
    PARAM: [type=boolean]                                [name=dynamic]                  [value=True or False]
    PARAM: [type={ref_link:Vector2}]                     [name=gravity_scale]            [value={ref_link:Vector2}]
    ATTR:  [type=function]                               [name={ref_link:add_child}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:get_child}]     [value=function] 
    ATTR:  [type=function]                               [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type=function]                               [name={ref_link:set_layer}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:get_layer}]     [value=function]
    ATTR:  [type=function]                               [name={ref_link:remove_child}]  [value=function]
    ATTR:  [type={ref_link:Vector2}]                     [name=position]                 [value={ref_link:Vector2}]
    ATTR:  [type={ref_link:PolygonCollisionShape2D}]     [name=collision_shape]          [value={ref_link:PolygonCollisionShape2D}]
    ATTR:  [type={ref_link:Vector2}]                     [name=velocity]                 [value={ref_link:Vector2}]
    ATTR:  [type={ref_link:Vector2}]                     [name=acceleration]             [value={ref_link:Vector2}]
    ATTR:  [type=float]                                  [name=rotation]                 [value=any]
    ATTR:  [type=float]                                  [name=mass]                     [value=any]
    ATTR:  [type=float]                                  [name=bounciness]               [value=any]
    ATTR:  [type=boolean]                                [name=dynamic]                  [value=True or False]
    ATTR:  [type={ref_link:Vector2}]                     [name=gravity_scale]            [value={ref_link:Vector2}]
    OVRR:  [type=function]                               [name={ref_link:tick}]          [value=function]
*/
mp_obj_t physics_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Physics2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_collision_shape,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_velocity,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_acceleration,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mass,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_bounciness,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dynamic,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_gravity_scale,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, collision_shape, velocity, acceleration, rotation, mass, bounciness, dynamic, gravity_scale};
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
    if(parsed_args[collision_shape].u_obj == MP_OBJ_NULL) parsed_args[collision_shape].u_obj = mp_const_none;
    if(parsed_args[velocity].u_obj == MP_OBJ_NULL) parsed_args[velocity].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[acceleration].u_obj == MP_OBJ_NULL) parsed_args[acceleration].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0);
    if(parsed_args[mass].u_obj == MP_OBJ_NULL) parsed_args[mass].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[bounciness].u_obj == MP_OBJ_NULL) parsed_args[bounciness].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[dynamic].u_obj == MP_OBJ_NULL) parsed_args[dynamic].u_obj = mp_obj_new_int(1);
    if(parsed_args[gravity_scale].u_obj == MP_OBJ_NULL) parsed_args[gravity_scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    engine_physics_2d_node_common_data_t *common_data = malloc(sizeof(engine_physics_2d_node_common_data_t));
    common_data->penetration = 0.0f;
    common_data->physics_id = engine_physics_take_available_id();

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_physics_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_PHYSICS_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    // Track the node base for this physics node so that it can
    // be looped over quickly in a linked list
    common_data->physics_list_node = engine_physics_track_node(node_base);

    if(inherited == false){        // Non-inherited (create a new object)
        engine_physics_2d_node_class_obj_t *physics_2d_node = m_malloc(sizeof(engine_physics_2d_node_class_obj_t));
        node_base->node = physics_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_draw_obj);
        common_data->collision_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_collision_obj);

        physics_2d_node->position = parsed_args[position].u_obj;
        physics_2d_node->collision_shape = parsed_args[collision_shape].u_obj;
        physics_2d_node->velocity = parsed_args[velocity].u_obj;
        physics_2d_node->acceleration = parsed_args[acceleration].u_obj;
        physics_2d_node->rotation = parsed_args[rotation].u_obj;
        physics_2d_node->mass = parsed_args[mass].u_obj;
        physics_2d_node->bounciness = parsed_args[bounciness].u_obj;
        physics_2d_node->dynamic = parsed_args[dynamic].u_obj;
        physics_2d_node->gravity_scale = parsed_args[gravity_scale].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->node = parsed_args[child_class].u_obj;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_collision, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->collision_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_collision_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->collision_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_collision_shape, parsed_args[collision_shape].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_velocity, parsed_args[velocity].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_acceleration, parsed_args[acceleration].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_mass, parsed_args[mass].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_bounciness, parsed_args[bounciness].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_dynamic, parsed_args[dynamic].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_gravity_scale, parsed_args[gravity_scale].u_obj);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


mp_obj_t physics_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Physics2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;
    engine_physics_untrack_node(common_data->physics_list_node);
    engine_physics_give_back_id(common_data->physics_id);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(physics_2d_node_class_del_obj, physics_2d_node_class_del);


STATIC void physics_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Physics2DNode attr");

    engine_physics_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&physics_2d_node_class_del_obj);
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
            case MP_QSTR_collision_shape:
                destination[0] = self->collision_shape;
            break;
            case MP_QSTR_velocity:
                destination[0] = self->velocity;
            break;
            case MP_QSTR_acceleration:
                destination[0] = self->acceleration;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_mass:
                destination[0] = self->mass;
            break;
            case MP_QSTR_bounciness:
                destination[0] = self->bounciness;
            break;
            case MP_QSTR_dynamic:
                destination[0] = self->dynamic;
            break;
            case MP_QSTR_gravity_scale:
                destination[0] = self->gravity_scale;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_collision_shape:
                self->collision_shape = destination[1];
            break;
            case MP_QSTR_velocity:
                self->velocity = destination[1];
            break;
            case MP_QSTR_acceleration:
                self->acceleration = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_mass:
                self->mass = destination[1];
            break;
            case MP_QSTR_bounciness:
                self->bounciness = destination[1];
            break;
            case MP_QSTR_dynamic:
                self->dynamic = destination[1];
            break;
            case MP_QSTR_gravity_scale:
                self->gravity_scale = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t physics_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(physics_2d_node_class_locals_dict, physics_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_physics_2d_node_class_type,
    MP_QSTR_Physics2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, physics_2d_node_class_new,
    print, physics_2d_node_class_print,
    attr, physics_2d_node_class_attr,
    locals_dict, &physics_2d_node_class_locals_dict
);