#include "physics_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "draw/engine_display_draw.h"
#include "physics/engine_physics.h"
#include "physics/shapes/engine_physics_shape_rectangle.h"
#include "physics/shapes/engine_physics_shape_circle.h"
#include "physics/shapes/engine_physics_shape_convex.h"
#include "physics/shapes/engine_physics_manifold.h"
#include "physics/shapes/shape_test.h"


// Class required functions
STATIC void physics_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Physics2DNode");
}


STATIC mp_obj_t physics_2d_node_class_tick(mp_obj_t self_in){
    // engine_node_base_t *node_base = self_in;
    // engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;

    // double x, y;

    // engine_physics_get_body_xy(common_data->physac_body, &x, &y);

    // ENGINE_WARNING_PRINTF("Physics2DNode: Tick function not overridden %0.3f %0.3f", x, y);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_2d_node_class_tick_obj, physics_2d_node_class_tick);

STATIC mp_obj_t physics_2d_node_class_apply_impulse(mp_obj_t self_in, mp_obj_t impulse_in, mp_obj_t point_in){

    const engine_node_base_t* self = self_in;
    vector2_class_obj_t* impulse = impulse_in;
    vector2_class_obj_t* point = point_in;

    mp_float_t i_mass = mp_obj_get_float(mp_load_attr(self->attr_accessor, MP_QSTR_i_mass));
    mp_float_t i_I = mp_obj_get_float(mp_load_attr(self->attr_accessor, MP_QSTR_i_I));
    mp_float_t ang_vel = mp_obj_get_float(mp_load_attr(self->attr_accessor, MP_QSTR_angular_velocity));

    vector2_class_obj_t* vel = MP_OBJ_TO_PTR(mp_load_attr(self->attr_accessor, MP_QSTR_velocity));
    vector2_class_obj_t* pos = MP_OBJ_TO_PTR(mp_load_attr(self->attr_accessor, MP_QSTR_position));

    vel->x += i_mass * impulse->x;
    vel->y += i_mass * impulse->y;

    ang_vel += i_I * ((point->x - pos->x) * impulse->y - (point->y - pos->y) * impulse->x);

    mp_store_attr(self->attr_accessor, MP_QSTR_angular_velocity, mp_obj_new_float(ang_vel));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(physics_2d_node_class_apply_impulse_obj, physics_2d_node_class_apply_impulse);

STATIC mp_obj_t physics_2d_node_class_apply_manifold_impulse(mp_obj_t a_in, mp_obj_t b_in, physics_manifold_class_obj_t* manifold){
    engine_node_base_t* a = a_in;
    engine_node_base_t* b = b_in;

    vector2_class_obj_t* a_pos = MP_OBJ_TO_PTR(mp_load_attr(a->attr_accessor, MP_QSTR_position));
    vector2_class_obj_t* b_pos = MP_OBJ_TO_PTR(mp_load_attr(b->attr_accessor, MP_QSTR_position));

    vector2_class_obj_t* a_vel = MP_OBJ_TO_PTR(mp_load_attr(a->attr_accessor, MP_QSTR_velocity));
    vector2_class_obj_t* b_vel = MP_OBJ_TO_PTR(mp_load_attr(b->attr_accessor, MP_QSTR_velocity));

    mp_float_t a_ang_vel = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_angular_velocity));
    mp_float_t b_ang_vel = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_angular_velocity));

    mp_float_t a_i_mass = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_i_mass));
    mp_float_t b_i_mass = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_i_mass));
    mp_float_t a_i_I = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_i_I));
    mp_float_t b_i_I = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_i_I));

    mp_float_t a_restitution = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_restitution));
    mp_float_t b_restitution = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_restitution));
    mp_float_t a_friction = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_friction));
    mp_float_t b_friction = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_friction));

    // Radii deltas
    mp_float_t rax = manifold->con_x - a_pos->x;
    mp_float_t ray = manifold->con_y - a_pos->y;

    mp_float_t rbx = manifold->con_x - a_pos->x;
    mp_float_t rby = manifold->con_y - a_pos->y;
    // Relative velocity
    mp_float_t rvx = b_vel->x - b_ang_vel * rbx - a_vel->x + a_ang_vel * rax;
    mp_float_t rvy = b_vel->y + b_ang_vel * rby - a_vel->y - a_ang_vel * ray;

    mp_float_t contact_vel = rvx * manifold->nrm_x + rvy * manifold->nrm_y;

    if(contact_vel > 0) return; // Separating

    // Mix restitution
    const mp_float_t e = MICROPY_FLOAT_C_FUN(fmax)(a_restitution, b_restitution);

    mp_float_t ra_cross_n = rax * manifold->nrm_y - ray * manifold->nrm_x;
    mp_float_t rb_cross_n = rbx * manifold->nrm_y - rby * manifold->nrm_x;
    mp_float_t i_mass_sum = a_i_mass + b_i_mass + ra_cross_n*ra_cross_n*a_i_I + rb_cross_n*rb_cross_n*b_i_I;
    mp_float_t j = -(1.0 + e) * contact_vel;
    j /= i_mass_sum;

    vector2_class_obj_t impulse_a = {{&vector2_class_type}, -manifold->nrm_x*j, -manifold->nrm_y*j};
    vector2_class_obj_t impulse_b = {{&vector2_class_type}, manifold->nrm_x*j, manifold->nrm_y*j};
    vector2_class_obj_t ra = {{&vector2_class_type}, rax, ray};
    vector2_class_obj_t rb = {{&vector2_class_type}, rbx, rby};
    // Apply normal impulses
    physics_2d_node_class_apply_impulse(a_in, MP_OBJ_FROM_PTR(&impulse_a), MP_OBJ_FROM_PTR(&ra));
    physics_2d_node_class_apply_impulse(b_in, MP_OBJ_FROM_PTR(&impulse_b), MP_OBJ_FROM_PTR(&rb));

    a_vel = MP_OBJ_TO_PTR(mp_load_attr(a->attr_accessor, MP_QSTR_velocity));
    b_vel = MP_OBJ_TO_PTR(mp_load_attr(b->attr_accessor, MP_QSTR_velocity));
    a_ang_vel = mp_obj_get_float(mp_load_attr(a->attr_accessor, MP_QSTR_angular_velocity));
    b_ang_vel = mp_obj_get_float(mp_load_attr(b->attr_accessor, MP_QSTR_angular_velocity));

    rvx = b_vel->x - b_ang_vel * rbx - a_vel->x + a_ang_vel * rax;
    rvy = b_vel->y + b_ang_vel * rby - a_vel->y - a_ang_vel * ray;

    mp_float_t rvdotn = rvx * manifold->nrm_x + rvy * manifold->nrm_y;

    vector2_class_obj_t t = {{&vector2_class_type}, rvx - rvdotn * manifold->nrm_x, rvy - rvdotn * manifold->nrm_y};

    mp_float_t t_il = 1.0 / MICROPY_FLOAT_C_FUN(sqrt)(t.x*t.x + t.y*t.y);

    t.x *= t_il;
    t.y *= t_il;

    mp_float_t jt = -(rvx*t.x + rvy*t.y);
    jt /= i_mass_sum;

    // Mix friction coefficients
    const mp_float_t f = MICROPY_FLOAT_C_FUN(sqrt)(a_friction * b_friction);

    vector2_class_obj_t t_impulse_a = {{&vector2_class_type}, -t.x*jt, -t.y*jt};
    vector2_class_obj_t t_impulse_b = {{&vector2_class_type}, t.x*jt, t.y*jt};

    physics_2d_node_class_apply_impulse(a_in, MP_OBJ_FROM_PTR(&t_impulse_a), MP_OBJ_FROM_PTR(&ra));
    physics_2d_node_class_apply_impulse(b_in, MP_OBJ_FROM_PTR(&t_impulse_b), MP_OBJ_FROM_PTR(&rb));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(physics_2d_node_class_apply_manifold_impulse_obj, physics_2d_node_class_apply_manifold_impulse);

STATIC mp_obj_t physics_2d_node_class_test(mp_obj_t self_in, mp_obj_t b_in){
    if(!mp_obj_is_type(self_in, &engine_physics_2d_node_class_type)){
        mp_raise_TypeError("expected physics node argument");
    }

    const engine_node_base_t* self = MP_OBJ_TO_PTR(self_in);
    const engine_node_base_t* b = MP_OBJ_TO_PTR(b_in);

    physics_manifold_class_obj_t* ret = physics_manifold_class_new(&physics_manifold_class_type, 0, 0, NULL);

    vector2_class_obj_t *a_pos = mp_load_attr(self->attr_accessor, MP_QSTR_position);
    vector2_class_obj_t *b_pos = mp_load_attr(b->attr_accessor, MP_QSTR_position);

    mp_obj_t a_shape = mp_load_attr(self->attr_accessor, MP_QSTR_physics_shape);
    mp_obj_t b_shape = mp_load_attr(b->attr_accessor, MP_QSTR_physics_shape);

    if(mp_obj_is_type(a_shape, &physics_shape_rectangle_class_type)){
        if(mp_obj_is_type(b_shape, &physics_shape_rectangle_class_type)){
            ENGINE_INFO_PRINTF("Physics2DNode: rectangle-rectangle test");
            physics_rectangle_rectangle_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else if(mp_obj_is_type(b_shape, &physics_shape_circle_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: rectangle-circle test");
            physics_circle_rectangle_test(b_pos, b_shape, a_pos, a_shape, ret);
            ret->mtv_x = -ret->mtv_x;
            ret->mtv_y = -ret->mtv_y;
            ret->nrm_x = -ret->nrm_x;
            ret->nrm_y = -ret->nrm_y;

        } else if(mp_obj_is_type(b_shape, &physics_shape_convex_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: rectangle-convex test");
            physics_convex_rectangle_test(b_pos, b_shape, a_pos, a_shape, ret);
            ret->mtv_x = -ret->mtv_x;
            ret->mtv_y = -ret->mtv_y;
            ret->nrm_x = -ret->nrm_x;
            ret->nrm_y = -ret->nrm_y;

        } else {
            mp_raise_TypeError("Unknown shape of B");

        }
    } else if(mp_obj_is_type(a_shape, &physics_shape_circle_class_type)) {
        if(mp_obj_is_type(b_shape, &physics_shape_rectangle_class_type)){
            ENGINE_INFO_PRINTF("Physics2DNode: circle-rectangle test");
            physics_circle_rectangle_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else if(mp_obj_is_type(b_shape, &physics_shape_circle_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: circle-circle test");
            physics_circle_circle_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else if(mp_obj_is_type(b_shape, &physics_shape_convex_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: circle-convex test");
            physics_convex_circle_test(b_pos, b_shape, a_pos, a_shape, ret);
            ret->mtv_x = -ret->mtv_x;
            ret->mtv_y = -ret->mtv_y;
            ret->nrm_x = -ret->nrm_x;
            ret->nrm_y = -ret->nrm_y;

        } else {
            mp_raise_TypeError("Unknown shape of B");

        }
    } else if(mp_obj_is_type(a_shape, &physics_shape_convex_class_type)) {
        if(mp_obj_is_type(b_shape, &physics_shape_rectangle_class_type)){
            ENGINE_INFO_PRINTF("Physics2DNode: convex-rectangle test");
            physics_convex_rectangle_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else if(mp_obj_is_type(b_shape, &physics_shape_circle_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: convex-circle test");
            physics_convex_circle_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else if(mp_obj_is_type(b_shape, &physics_shape_convex_class_type)) {
            ENGINE_INFO_PRINTF("Physics2DNode: convex-convex test");
            physics_convex_convex_test(a_pos, a_shape, b_pos, b_shape, ret);

        } else {
            mp_raise_TypeError("Unknown shape of B");

        }
    } else {
        mp_raise_TypeError("Unknown shape of A");
    }

    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_2d_node_class_test_obj, physics_2d_node_class_test);

STATIC mp_obj_t physics_2d_node_class_compute_mass(mp_obj_t self_in, mp_obj_t density_in){
    if(!mp_obj_is_type(self_in, &engine_physics_2d_node_class_type)){
        mp_raise_TypeError("expected physics node argument");
    }

    const engine_node_base_t* self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t a_shape = mp_load_attr(self->attr_accessor, MP_QSTR_physics_shape);
    const mp_float_t density = mp_obj_get_float(density_in);

    if(mp_obj_is_type(a_shape, &physics_shape_rectangle_class_type)){
        physics_shape_rectangle_class_obj_t* rect = MP_OBJ_TO_PTR(a_shape);
        mp_float_t mass = rect->width * rect->height * density;
        mp_store_attr(self->attr_accessor, MP_QSTR_i_mass, mp_obj_new_float(1.0/mass));
        mp_store_attr(self->attr_accessor, MP_QSTR_i_I, mp_obj_new_float(1.0));
    } else if(mp_obj_is_type(a_shape, &physics_shape_circle_class_type)) {
        physics_shape_circle_class_obj_t* c = MP_OBJ_TO_PTR(a_shape);
        mp_float_t r2 = c->radius * c->radius;
        mp_float_t mass = M_PI * r2 * density;
        mp_store_attr(self->attr_accessor, MP_QSTR_i_mass, mp_obj_new_float(1.0/mass));
        mp_store_attr(self->attr_accessor, MP_QSTR_i_I, mp_obj_new_float(1.0/(M_PI*r2*r2)));
    } else if(mp_obj_is_type(a_shape, &physics_shape_convex_class_type)) {
        physics_shape_convex_class_obj_t* convex = MP_OBJ_TO_PTR(a_shape);
        mp_float_t mass = convex->area * density;
        mp_store_attr(self->attr_accessor, MP_QSTR_i_mass, mp_obj_new_float(1.0/mass));
        mp_store_attr(self->attr_accessor, MP_QSTR_i_I, mp_obj_new_float(1.0/(convex->I)));
    } else {
        mp_raise_TypeError("Unknown shape of A");
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_2d_node_class_compute_mass_obj, physics_2d_node_class_compute_mass);


STATIC mp_obj_t physics_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Physics2DNode: Drawing");

    engine_node_base_t* base = self_in;

    // engine_node_base_t *node_base = self_in;
    // engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;

    // PhysicsBody body = common_data->physac_body;

    // int vertexCount = engine_physics_get_vertex_count(common_data->physac_body);
    // for(int j = 0; j < vertexCount; j++){
    //     float vertex_a_x;
    //     float vertex_a_y;
    //     float vertex_b_x;
    //     float vertex_b_y;

    //     engine_physics_get_vertex(common_data->physac_body, &vertex_a_x, &vertex_a_y, j);

    //     int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);
    //     engine_physics_get_vertex(common_data->physac_body, &vertex_b_x, &vertex_b_y, jj);

    //     engine_draw_line(0b0000000000011111, vertex_a_x, vertex_a_y, vertex_b_x, vertex_b_y, camera_node);

    //     // Get physics bodies shape vertices to draw lines
    //     // Note: GetPhysicsShapeVertex() already calculates rotation transformations
    //     // Vector2 vertexA = GetPhysicsShapeVertex(body, j);

    //     // int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
    //     // Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

    //     // DrawLineV(vertexA, vertexB, GREEN);     // Draw a line between two vertex positions
    // }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(physics_2d_node_class_draw_obj, physics_2d_node_class_draw);


mp_obj_t physics_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Physics2DNode");

    engine_physics_2d_node_common_data_t *common_data = malloc(sizeof(engine_physics_2d_node_common_data_t));

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
    // be looped over quickly in a linked list and have its
    // attributes copied back and forth quickly between
    // the engine and physics engine
    common_data->physics_list_node = engine_physics_track_node(node_base);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_physics_2d_node_class_obj_t *physics_2d_node = m_malloc(sizeof(engine_physics_2d_node_class_obj_t));
        node_base->node = physics_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_draw_obj);

        physics_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->rotation = 0.0;
        physics_2d_node->velocity = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->acceleration = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->dynamic = mp_obj_new_bool(true);
        physics_2d_node->physics_shape = mp_const_none;
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
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

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_velocity, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_acceleration, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_dynamic, mp_obj_new_bool(true));
        mp_store_attr(node_base->node, MP_QSTR_physics_shape, mp_const_none);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to Physics2DNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC void physics_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Physics2DNode attr");

    engine_physics_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_add_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
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
            case MP_QSTR_test:
                destination[0] = MP_OBJ_FROM_PTR(&physics_2d_node_class_test_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_compute_mass:
                destination[0] = MP_OBJ_FROM_PTR(&physics_2d_node_class_compute_mass_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_apply_impulse:
                destination[0] = MP_OBJ_FROM_PTR(&physics_2d_node_class_apply_impulse_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_apply_manifold_impulse:
                destination[0] = MP_OBJ_FROM_PTR(&physics_2d_node_class_apply_manifold_impulse_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_rotation:
                destination[0] = mp_obj_new_float(self->rotation);
            break;
            case MP_QSTR_angular_velocity:
                destination[0] = mp_obj_new_float(self->angular_velocity);
            break;
            case MP_QSTR_i_mass:
                destination[0] = mp_obj_new_float(self->i_mass);
            break;
            case MP_QSTR_i_I:
                destination[0] = mp_obj_new_float(self->i_I);
            break;
            case MP_QSTR_restitution:
                destination[0] = mp_obj_new_float(self->restitution);
            break;
            case MP_QSTR_friction:
                destination[0] = mp_obj_new_float(self->friction);
            break;
            case MP_QSTR_velocity:
                destination[0] = self->velocity;
            break;
            case MP_QSTR_acceleration:
                destination[0] = self->acceleration;
            break;
            case MP_QSTR_dynamic:
                destination[0] = self->dynamic;
            break;
            case MP_QSTR_physics_shape:
                destination[0] = self->physics_shape;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_angular_velocity:
                self->angular_velocity = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_i_mass:
                self->i_mass = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_i_I:
                self->i_I = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_restitution:
                self->restitution = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_friction:
                self->friction = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_velocity:
                self->velocity = destination[1];
            break;
            case MP_QSTR_acceleration:
                self->acceleration = destination[1];
            break;
            case MP_QSTR_dynamic:
                self->dynamic = destination[1];
            break;
            case MP_QSTR_physics_shape:
                self->physics_shape = destination[1];
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


// Class init
STATIC MP_DEFINE_CONST_DICT(physics_2d_node_class_locals_dict, physics_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_physics_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Physics2DNode,
    .print = physics_2d_node_class_print,
    .make_new = physics_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = physics_2d_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&physics_2d_node_class_locals_dict,
};
