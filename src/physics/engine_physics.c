#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2D/physics_rectangle_2d_node.h"
#include "nodes/2D/physics_circle_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "utility/engine_bit_collection.h"
#include "collision_contact_2d.h"
#include "nodes/node_types.h"
#include "py/obj.h"
#include "draw/engine_display_draw.h"
#include "physics/engine_physics_ids.h"
#include "physics/engine_physics_collision.h"
#include "utility/engine_time.h"
#include "engine.h"
#include "engine_collections.h"
#include "engine_physics_module.h"

// Bit array/collection to track nodes that have collided. In the `init` function
// this is sized so that the output indices from a simple paring function can fit
// (https://math.stackexchange.com/a/531914)
engine_bit_collection_t collided_physics_nodes;

const float slop = 0.1f;   // usually 0.01 to 0.1


// https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#timestepping:~:text=Here%20is%20a%20full%20example%3A
float time_accumulator = 0.0f;
uint32_t frame_start_ms = 0;


void engine_physics_init(){
    ENGINE_INFO_PRINTF("EnginePhysics: Starting...")
    engine_physics_ids_init();
    engine_bit_collection_create(&collided_physics_nodes, engine_physics_ids_get_pair_index(PHYSICS_ID_MAX, PHYSICS_ID_MAX));
    frame_start_ms = millis();
}


void engine_physics_apply_impulses(float dt, float alpha){
    vector2_class_obj_t *gravity = engine_physics_get_gravity();

    linked_list *physics_list = engine_collections_get_physics_list();
    linked_list_node *physics_link_node = physics_list->start;

    while(physics_link_node != NULL){
        engine_node_base_t *node_base = physics_link_node->object;
        engine_physics_node_base_t *physics_node_base = node_base->node;

        bool physics_node_dynamic = mp_obj_get_int(physics_node_base->dynamic);

        if(physics_node_dynamic){
            vector2_class_obj_t *physics_node_velocity = physics_node_base->velocity;
            vector2_class_obj_t *physics_node_position = physics_node_base->position;
            vector2_class_obj_t *physics_node_gravity_scale = physics_node_base->gravity_scale;

            // Position correction
            physics_node_position->x.value += physics_node_base->total_position_correction_x;
            physics_node_position->y.value += physics_node_base->total_position_correction_y;

            physics_node_base->total_position_correction_x = 0.0f;
            physics_node_base->total_position_correction_y = 0.0f;

            // Gravity: https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Scene.cpp#L35-L42
            //          https://github.com/victorfisac/Physac/blob/29d9fc06860b54571a02402fff6fa8572d19bd12/src/physac.h#L1644-L1648

            physics_node_velocity->x.value -= gravity->x.value * physics_node_gravity_scale->x.value;
            physics_node_velocity->y.value -= gravity->y.value * physics_node_gravity_scale->y.value;

            // Velocity -> position: https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Scene.cpp#L44-L53
            physics_node_position->x.value += physics_node_velocity->x.value;
            physics_node_position->y.value += physics_node_velocity->y.value;

            physics_node_base->rotation += physics_node_base->angular_velocity;
        }

        // If the node was colliding last tick but isn't now, call the on_separate_cb callback
        if(physics_node_base->on_separate_cb != mp_const_none && physics_node_base->was_colliding == true && physics_node_base->colliding == false){
            mp_call_method_n_kw(0, 0, (mp_obj_t[]){physics_node_base->on_separate_cb, node_base->attr_accessor});
        }

        physics_link_node = physics_link_node->next;
    }
}


bool engine_physics_collision_checked_before(engine_physics_node_base_t *physics_node_base_a, engine_physics_node_base_t *physics_node_base_b){
    // Sort so that id_max*a_id + b_id comes out
    // the same no matter which node is `a` or `b`
    // Get the perfect hash code (can be perfect
    // since the range is limited)
    uint32_t pair_index = 0;

    if(physics_node_base_a->physics_id > physics_node_base_b->physics_id){
        pair_index = engine_physics_ids_get_pair_index(physics_node_base_b->physics_id, physics_node_base_a->physics_id);
    }else{
        pair_index = engine_physics_ids_get_pair_index(physics_node_base_a->physics_id, physics_node_base_b->physics_id);
    }

    bool checked_before = engine_bit_collection_get(&collided_physics_nodes, pair_index);   // Get if checked before
    engine_bit_collection_set(&collided_physics_nodes, pair_index);                         // Set that it has/will be checked no matter what
    return checked_before;
}


void engine_physics_collide_types(engine_node_base_t *node_base_a, engine_node_base_t *node_base_b){
    engine_physics_node_base_t *physics_node_base_a = node_base_a->node;
    engine_physics_node_base_t *physics_node_base_b = node_base_b->node;

    // Do not try to collide nodes that are not on the same collision 'layer'
    if((physics_node_base_a->collision_mask & physics_node_base_b->collision_mask) == 0){
        return;
    }

    // Before we got here it was confirmed that we're not
    // checking for collision between the same object, now
    // confirm these individual objects have not been checked
    // for collision before
    if(engine_physics_collision_checked_before(physics_node_base_a, physics_node_base_b)){
        return;
    }

    physics_contact_t contact;
    engine_physics_setup_contact(&contact);

    bool collided = false;

    // Now that it has been confirmed that the two objects are
    // not the same object and that they have not been checked
    // for collision before, check them now but make sure to
    // check the correct pairing (rect vs. rect, rect vs. circle,
    // or circle vs. circle)
    if(node_base_a->type == NODE_TYPE_PHYSICS_RECTANGLE_2D && node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){
        physics_abs_rectangle_t abs_rect_a;
        physics_abs_rectangle_t abs_rect_b;
        engine_physics_setup_abs_rectangle(node_base_a, &abs_rect_a);
        engine_physics_setup_abs_rectangle(node_base_b, &abs_rect_b);

        collided = engine_physics_check_rect_rect_collision(&abs_rect_a, &abs_rect_b, &contact);
    }else if((node_base_a->type == NODE_TYPE_PHYSICS_RECTANGLE_2D && node_base_b->type == NODE_TYPE_PHYSICS_CIRCLE_2D) ||
             (node_base_a->type == NODE_TYPE_PHYSICS_CIRCLE_2D    && node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D)){

        // Want `physics_node_base_a` to always be the rectangle
        // and `physics_node_base_b` to be the circle
        if(node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){
            engine_node_base_t *temp = node_base_a;
            node_base_a = node_base_b;
            node_base_b = temp;

            physics_node_base_a = node_base_a->node;
            physics_node_base_b = node_base_b->node;
        }

        physics_abs_rectangle_t abs_rect;
        physics_abs_circle_t abs_circle;
        engine_physics_setup_abs_rectangle(node_base_a, &abs_rect);
        engine_physics_setup_abs_circle(node_base_b, &abs_circle);

        collided = engine_physics_check_rect_circle_collision(&abs_rect, &abs_circle, &contact);
    }else if(node_base_a->type == NODE_TYPE_PHYSICS_CIRCLE_2D && node_base_b->type == NODE_TYPE_PHYSICS_CIRCLE_2D){
        physics_abs_circle_t abs_circle_a;
        physics_abs_circle_t abs_circle_b;
        engine_physics_setup_abs_circle(node_base_a, &abs_circle_a);
        engine_physics_setup_abs_circle(node_base_b, &abs_circle_b);

        collided = engine_physics_check_circle_circle_collision(&abs_circle_a, &abs_circle_b, &contact);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Unknown collider pair collision check!"));
    }

    if(collided){
        // This pair is colliding, mark each as so (this
        // flag just means "colliding with something")
        physics_node_base_a->colliding = true;
        physics_node_base_b->colliding = true;

        bool physics_node_a_dynamic = mp_obj_get_int(physics_node_base_a->dynamic);
        bool physics_node_b_dynamic = mp_obj_get_int(physics_node_base_b->dynamic);

        bool physics_node_a_solid = mp_obj_get_int(physics_node_base_a->solid);
        bool physics_node_b_solid = mp_obj_get_int(physics_node_base_b->solid);

        // Calculate restitution/bounciness
        float physics_node_a_bounciness = mp_obj_get_float(physics_node_base_a->bounciness);
        float physics_node_b_bounciness = mp_obj_get_float(physics_node_base_b->bounciness);
        // float bounciness = (physics_node_a_bounciness+physics_node_b_bounciness) * 0.5f; // Restitution: https://github.com/victorfisac/Physac/blob/29d9fc06860b54571a02402fff6fa8572d19bd12/src/physac.h#L1664
        float bounciness = sqrtf(physics_node_a_bounciness*physics_node_b_bounciness);

        // if(engine_math_vector_length_sqr(contact.relative_velocity_x, contact.relative_velocity_y) < engine_math_vector_length_sqr(engine_physics_gravity_x, engine_physics_gravity_y) + EPSILON){
        //     bounciness = 0.0f;
        // }

        // float bounciness = fminf(physics_node_a_bounciness, physics_node_b_bounciness);

        // // https://github.com/RandyGaul/ImpulseEngine/blob/master/Manifold.cpp#L65-L92
        // // https://github.com/victorfisac/Physac/blob/29d9fc06860b54571a02402fff6fa8572d19bd12/src/physac.h#L1726
        // float cross_a = engine_math_cross_product_v_v(contact.moment_arm_a_x, contact.moment_arm_a_y, contact.collision_normal_x, contact.collision_normal_y);
        // float cross_b = engine_math_cross_product_v_v(contact.moment_arm_b_x, contact.moment_arm_b_y, contact.collision_normal_x, contact.collision_normal_y);
        // float inv_mass_sum = physics_node_base_a->inverse_mass + physics_node_base_b->inverse_mass + (cross_a*cross_a) * physics_node_base_a->inverse_moment_of_inertia + (cross_b*cross_b) * physics_node_base_b->inverse_moment_of_inertia;

        // float j = -(1.0f + bounciness) * contact.contact_velocity_magnitude;
        // j /= inv_mass_sum;

        // float impulse_x = contact.collision_normal_x * j;
        // float impulse_y = contact.collision_normal_y * j;

        // if(physics_node_a_dynamic) physics_node_base_apply_impulse_base(physics_node_base_a, -impulse_x, -impulse_y, contact.moment_arm_a_x, contact.moment_arm_a_y);
        // if(physics_node_b_dynamic) physics_node_base_apply_impulse_base(physics_node_base_b,  impulse_x,  impulse_y, contact.moment_arm_b_x, contact.moment_arm_b_y);


        vector2_class_obj_t *physics_node_a_velocity = physics_node_base_a->velocity;
        vector2_class_obj_t *physics_node_b_velocity = physics_node_base_b->velocity;

        float inv_mass_sum = physics_node_base_a->inverse_mass + physics_node_base_b->inverse_mass;

        // Impulse due to nodes' velocities and collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=Calculate%20impulse%20scalar
        float collision_impulse_j = -(1.0f + bounciness) * contact.contact_velocity_magnitude;
        collision_impulse_j /= inv_mass_sum;

        float collision_impulse_x = collision_impulse_j * contact.collision_normal_x;
        float collision_impulse_y = collision_impulse_j * contact.collision_normal_y;

        // Impulse due to overlap during collision detection: https://gamedev.stackexchange.com/a/114793
        float separate_impulse_x = 0.0f;
        float separate_impulse_y = 0.0f;

        if(contact.collision_normal_penetration > slop){
            float separate_impulse_j = -contact.collision_normal_penetration * 0.03f;

            separate_impulse_x = separate_impulse_j * contact.collision_normal_x;
            separate_impulse_y = separate_impulse_j * contact.collision_normal_y;
        }

        // Apply impulses to linear/positional velocity
        if(physics_node_a_solid && physics_node_b_solid){
            float correction_ratio = 0.7f * contact.collision_normal_penetration / inv_mass_sum;

            if(physics_node_a_dynamic){
                physics_node_base_a->total_position_correction_x += contact.collision_normal_x * correction_ratio * physics_node_base_a->inverse_mass;
                physics_node_base_a->total_position_correction_y += contact.collision_normal_y * correction_ratio * physics_node_base_a->inverse_mass;

                physics_node_a_velocity->x.value -= physics_node_base_a->inverse_mass * (collision_impulse_x + separate_impulse_x);
                physics_node_a_velocity->y.value -= physics_node_base_a->inverse_mass * (collision_impulse_y + separate_impulse_y);
            }

            if(physics_node_b_dynamic){
                physics_node_base_b->total_position_correction_x -= contact.collision_normal_x * correction_ratio * physics_node_base_b->inverse_mass;
                physics_node_base_b->total_position_correction_y -= contact.collision_normal_y * correction_ratio * physics_node_base_b->inverse_mass;

                physics_node_b_velocity->x.value += physics_node_base_b->inverse_mass * (collision_impulse_x + separate_impulse_x);
                physics_node_b_velocity->y.value += physics_node_base_b->inverse_mass * (collision_impulse_y + separate_impulse_y);
            }
        }

        // if(contact.collision_normal_penetration > slop){
        //     float correction_x = contact.collision_normal_penetration * contact.collision_normal_x;
        //     float correction_y = contact.collision_normal_penetration * contact.collision_normal_y;

        //     // Using the normalized collision normal, offset positions of
        //     // both nodes by the amount they were overlapping (in pixels)
        //     // when the collision was detected. Split the overlap 50/50
        //     //
        //     // Depending on which objects are dynamic, move the dynamic bodies by
        //     // the penetration amount. Don't want static nodes to be moved by the
        //     // penetration amount.
        //     if(physics_node_a_dynamic == true && physics_node_b_dynamic == false){
        //         physics_node_base_a->total_position_correction_x += correction_x;
        //         physics_node_base_a->total_position_correction_y += correction_y;
        //     }else if(physics_node_a_dynamic == false && physics_node_b_dynamic == true){
        //         physics_node_base_b->total_position_correction_x -= correction_x;
        //         physics_node_base_b->total_position_correction_y -= correction_y;
        //     }else if(physics_node_a_dynamic == true && physics_node_b_dynamic == true){
        //         physics_node_base_a->total_position_correction_x += correction_x / 2;
        //         physics_node_base_a->total_position_correction_y += correction_y / 2;

        //         physics_node_base_b->total_position_correction_x -= correction_x / 2;
        //         physics_node_base_b->total_position_correction_y -= correction_y / 2;
        //     }
        // }




        // // https://gamedev.stackexchange.com/a/102778
        // float contact_point_sqrd_length = ((contact.collision_contact_x*contact.collision_contact_x) + (contact.collision_contact_y*contact.collision_contact_y));
        // physics_node_base_a->angular_velocity += physics_node_base_a->inverse_moment_of_inertia * engine_math_cross_product_v_v(contact.collision_contact_x, contact.collision_contact_x, physics_node_a_velocity->x, physics_node_a_velocity->y) / contact_point_sqrd_length;
        // physics_node_base_b->angular_velocity += physics_node_base_b->inverse_moment_of_inertia * engine_math_cross_product_v_v(contact.collision_contact_x, contact.collision_contact_x, physics_node_b_velocity->x, physics_node_b_velocity->y) / contact_point_sqrd_length;


        // Friction: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-friction-scene-and-jump-table--gamedev-7756t#:~:text=in%20our%20collision%20resolver
        float a_friction = mp_obj_get_float(physics_node_base_a->friction);
        float b_friction = mp_obj_get_float(physics_node_base_b->friction);
        float mu = sqrtf(a_friction + b_friction);

        contact.relative_velocity_x = physics_node_b_velocity->x.value - physics_node_a_velocity->x.value;
        contact.relative_velocity_y = physics_node_b_velocity->y.value - physics_node_a_velocity->y.value;

        // engine_physics_get_relative_velocity(physics_node_base_a, physics_node_base_b, &contact);

        float dot = engine_math_dot_product(contact.relative_velocity_x, contact.relative_velocity_y, contact.collision_normal_x, contact.collision_normal_y);
        float tangent_x = contact.relative_velocity_x - dot * contact.collision_normal_x;
        float tangent_y = contact.relative_velocity_y - dot * contact.collision_normal_y;
        engine_math_normalize(&tangent_x, &tangent_y);

        float jt = -engine_math_dot_product(contact.relative_velocity_x, contact.relative_velocity_y, tangent_x, tangent_y);
        jt /= inv_mass_sum;

        // Don't apply very small friction impulses
        if(engine_math_compare_floats(jt, 0.0f) == false){
            float friction_impulse_x = tangent_x * jt * mu;
            float friction_impulse_y = tangent_y * jt * mu;

            // physics_node_base_apply_impulse_base(physics_node_base_a, -friction_impulse_x, -friction_impulse_y, contact.moment_arm_a_x, contact.moment_arm_a_y);
            // physics_node_base_apply_impulse_base(physics_node_base_b,  friction_impulse_x,  friction_impulse_y, contact.moment_arm_b_x, contact.moment_arm_b_y);

            if(physics_node_a_solid && physics_node_b_solid){
                if(physics_node_a_dynamic){
                    physics_node_a_velocity->x.value -= physics_node_base_a->inverse_mass * friction_impulse_x;
                    physics_node_a_velocity->y.value -= physics_node_base_a->inverse_mass * friction_impulse_y;
                }

                if(physics_node_b_dynamic){
                    physics_node_b_velocity->x.value += physics_node_base_b->inverse_mass * friction_impulse_x;
                    physics_node_b_velocity->y.value += physics_node_base_b->inverse_mass * friction_impulse_y;
                }
            }
        }

        mp_obj_t collision_contact_data[5];
        collision_contact_data[0] = mp_obj_new_float(contact.collision_contact_x);
        collision_contact_data[1] = mp_obj_new_float(contact.collision_contact_y);
        collision_contact_data[2] = mp_obj_new_float(contact.collision_normal_x);
        collision_contact_data[3] = mp_obj_new_float(contact.collision_normal_y);

        mp_obj_t exec[3];

        // Call A callback
        if(physics_node_base_a->on_collide_cb != mp_const_none){
            collision_contact_data[4] = node_base_b;
            exec[0] = physics_node_base_a->on_collide_cb;
            exec[1] = node_base_a->attr_accessor;
            exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
            mp_call_method_n_kw(1, 0, exec);
        }

        // Call B callback
        if(physics_node_base_b->on_collide_cb != mp_const_none){
            collision_contact_data[4] = node_base_a;
            exec[0] = physics_node_base_b->on_collide_cb;
            exec[1] = node_base_b->attr_accessor;
            exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
            mp_call_method_n_kw(1, 0, exec);
        }
    }
}


void engine_physics_update(float dt){
    // Loop through all nodes and test for collision against
    // all other nodes (not optimized checking of if nodes are
    // even possibly close to each other)
    linked_list *physics_list = engine_collections_get_physics_list();
    linked_list_node *physics_link_node_a = physics_list->start;
    while(physics_link_node_a != NULL){
        // Now check 'a' against all nodes 'b'
        linked_list_node *physics_link_node_b = physics_list->start;

        while(physics_link_node_b != NULL){
            // Make sure we are not checking against ourselves
            if(physics_link_node_a->object != physics_link_node_b->object){
                engine_physics_collide_types(physics_link_node_a->object, physics_link_node_b->object);
            }

            physics_link_node_b = physics_link_node_b->next;
        }

        // Now that `a` was compared to all `b`,
        // move onto the next `a` and do it again
        physics_link_node_a = physics_link_node_a->next;
    }

    // After everything physics related is done, reset the bit array
    // used for tracking which pairs of nodes had already collided
    engine_bit_collection_erase(&collided_physics_nodes);
}


void engine_physics_physics_tick(float dt_s){
    mp_obj_t exec[3];

    // Loop through all nodes and call their physics_tick callbacks
    linked_list *physics_list = engine_collections_get_physics_list();
    linked_list_node *physics_link_node = physics_list->start;
    while(physics_link_node != NULL){
        engine_node_base_t *node_base = physics_link_node->object;
        engine_physics_node_base_t *physics_node_base = node_base->node;

        if(physics_node_base->physics_tick_cb != mp_const_none){
            exec[0] = physics_node_base->physics_tick_cb;
            exec[1] = node_base->attr_accessor;
            exec[2] = mp_obj_new_float(dt_s);
            mp_call_method_n_kw(1, 0, exec);
        }

        // Before setting to back to false, track the colliding state
        physics_node_base->was_colliding = physics_node_base->colliding;

        // Set this to false so that it can be
        // set back to true only if colliding, next
        physics_node_base->colliding = false;

        physics_link_node = physics_link_node->next;
    }
}


void engine_physics_tick(){
    // https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#timestepping:~:text=Here%20is%20a%20full%20example%3A
    float engine_fps_limit_period_ms = engine_get_fps_limit_ms();

    const float alpha = time_accumulator / engine_fps_limit_period_ms;

    const uint32_t current_time_ms = millis();

    // Store the time elapsed since the last frame began
    time_accumulator += millis_diff(current_time_ms, frame_start_ms);

    // Record the starting of this frame
    frame_start_ms = current_time_ms;

    // Avoid spiral of death and clamp dt, thus clamping
    // how many times the update physics can be called in
    // a single game loop.
    if(time_accumulator > 30.0f){
        time_accumulator = 30.0f;
    }

    while(time_accumulator > engine_fps_limit_period_ms){
        // Call the physics_tick callbacks on all physics nodes first
        engine_physics_physics_tick(engine_fps_limit_period_ms);

        engine_physics_update(engine_fps_limit_period_ms);
        time_accumulator -= engine_fps_limit_period_ms;

        // Apply impulses/move objects due to physics before
        // checking for collisions. Doing it this way means
        // you don't see when objects are overlapping and moved
        // back (looks more stable)
        engine_physics_apply_impulses(engine_fps_limit_period_ms, alpha);
    }
}