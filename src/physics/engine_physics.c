#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2d/physics_rectangle_2d_node.h"
#include "nodes/2d/physics_circle_2d_node.h"
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

// Bit array/collection to track nodes that have collided. In the `init` function
// this is sized so that the output indices from a simple paring function can fit
// (https://math.stackexchange.com/a/531914)
engine_bit_collection_t collided_physics_nodes;


linked_list engine_physics_nodes;
float engine_physics_gravity_x = 0.0f;
float engine_physics_gravity_y = -0.00981f;

const float percent = 0.9f; // usually 20% to 80% 
const float slop = 0.01f;   // usually 0.01 to 0.1


// https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#timestepping:~:text=Here%20is%20a%20full%20example%3A
float time_accumulator = 0.0f;
float frame_start_ms = 0.0f;


void engine_physics_init(){
    ENGINE_INFO_PRINTF("EnginePhysics: Starting...")
    engine_physics_ids_init();
    engine_bit_collection_create(&collided_physics_nodes, engine_physics_ids_get_pair_index(PHYSICS_ID_MAX, PHYSICS_ID_MAX));
    frame_start_ms = millis();
}


void engine_physics_apply_impulses(float dt, float alpha){
    linked_list_node *physics_link_node = engine_physics_nodes.start;
    while(physics_link_node != NULL){
        engine_node_base_t *node_base = physics_link_node->object;
        engine_physics_node_base_t *physics_node_base = node_base->node;

        bool physics_node_dynamic = mp_obj_get_int(physics_node_base->dynamic);

        if(physics_node_dynamic){
            vector2_class_obj_t *physics_node_velocity = physics_node_base->velocity;
            float physics_node_angular_velocity = mp_obj_get_float(physics_node_base->angular_velocity);
            vector2_class_obj_t *physics_node_position = physics_node_base->position;
            vector2_class_obj_t *physics_node_gravity_scale = physics_node_base->gravity_scale;

            // https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Scene.cpp#L35-L42
            physics_node_velocity->x -= physics_node_base->inverse_mass * engine_physics_gravity_x * physics_node_gravity_scale->x;
            physics_node_velocity->y -= physics_node_base->inverse_mass * engine_physics_gravity_y * physics_node_gravity_scale->y;

            physics_node_position->x += physics_node_base->total_position_correction_x;
            physics_node_position->y += physics_node_base->total_position_correction_y;

            physics_node_base->total_position_correction_x = 0.0f;
            physics_node_base->total_position_correction_y = 0.0f;


            // https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Scene.cpp#L44-L53
            physics_node_position->x += physics_node_velocity->x;
            physics_node_position->y += physics_node_velocity->y;

            physics_node_base->rotation += physics_node_angular_velocity;

            // When the rotation changes the collider box vertices need to be updated
            if(node_base->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){
                engine_physics_rectangle_2d_node_update(physics_node_base);
            }
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

    // Before we got here it was confirmed that we're not
    // checking for collision between the same object, now
    // confirm these individual objects have not been checked
    // for collision before
    if(engine_physics_collision_checked_before(physics_node_base_a, physics_node_base_b)){
        return;
    }

    contact_t contact = {
        .collision_normal_x = 0.0f,
        .collision_normal_y = 0.0f,
        .collision_contact_x = 0.0f,
        .collision_contact_y = 0.0f,
        .collision_normal_penetration = 0.0f,
        .relative_velocity_x = 0.0f,
        .relative_velocity_y = 0.0f,
        .contact_velocity_magnitude = 0.0f
    };

    bool collided = false;

    // Now that it has been confirmed that the two objects are
    // not the same object and that they have not been checked
    // for collision before, check them now but make sure to
    // check the correct pairing (rect vs. rect, rect vs. circle,
    // or circle vs. circle)
    if(node_base_a->type == NODE_TYPE_PHYSICS_RECTANGLE_2D && node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){
        collided = engine_physics_check_rect_rect_collision(physics_node_base_a, physics_node_base_b, &contact);
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

        collided = engine_physics_check_rect_circle_collision(physics_node_base_a, physics_node_base_b, &contact);
    }else if(node_base_a->type == NODE_TYPE_PHYSICS_CIRCLE_2D && node_base_b->type == NODE_TYPE_PHYSICS_CIRCLE_2D){
        collided = engine_physics_check_circle_circle_collision(physics_node_base_a, physics_node_base_b, &contact);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Unknown collider pair collision check!"));
    }

    if(collided){
        vector2_class_obj_t *physics_node_a_velocity = physics_node_base_a->velocity;
        vector2_class_obj_t *physics_node_b_velocity = physics_node_base_b->velocity;

        vector2_class_obj_t *physics_node_a_position = physics_node_base_a->position;
        vector2_class_obj_t *physics_node_b_position = physics_node_base_b->position;

        bool physics_node_a_dynamic = mp_obj_get_int(physics_node_base_a->dynamic);
        bool physics_node_b_dynamic = mp_obj_get_int(physics_node_base_b->dynamic);

        // Calculate restitution/bounciness
        float physics_node_a_bounciness = mp_obj_get_float(physics_node_base_a->bounciness);
        float physics_node_b_bounciness = mp_obj_get_float(physics_node_base_b->bounciness);
        float bounciness = fminf(physics_node_a_bounciness, physics_node_b_bounciness); // Restitution



        // float ra_x = contact.collision_contact_x - physics_node_a_position->x;
        // float ra_y = contact.collision_contact_y - physics_node_a_position->y;

        // float rb_x = contact.collision_contact_x - physics_node_b_position->x;
        // float rb_y = contact.collision_contact_y - physics_node_b_position->y;


        // float cross_a = engine_math_cross_product_v_v(ra_x, ra_y, contact.collision_normal_x, contact.collision_normal_y);
        // float cross_b = engine_math_cross_product_v_v(rb_x, rb_y, contact.collision_normal_x, contact.collision_normal_y);

        // float inv_inertia_sum = sqrtf(cross_a) * physics_node_base_a->inverse_moment_of_inertia + sqrtf(cross_b) * physics_node_base_b->inverse_moment_of_inertia;
        float inv_mass_sum = physics_node_base_a->inverse_mass + physics_node_base_b->inverse_mass;

        float impulse_coefficient_j = -(1 + bounciness) * contact.contact_velocity_magnitude;
        
        float impulse_coefficient_j_mass = impulse_coefficient_j / inv_mass_sum;
        // float impulse_coefficient_j_inertia = impulse_coefficient_j / inv_mass_sum;

        float impulse_x = impulse_coefficient_j_mass * contact.collision_normal_x;
        float impulse_y = impulse_coefficient_j_mass * contact.collision_normal_y;

        // physics_node_base_a->angular_velocity = mp_obj_new_float(-impulse_coefficient_j * impulse_coefficient_j_inertia);
        // physics_node_base_b->angular_velocity = mp_obj_new_float( impulse_coefficient_j * impulse_coefficient_j_inertia);

        // physics_node_base_apply_impulse_base(physics_node_base_a, impulse_x, impulse_y, ra_x, ra_y);
        // physics_node_base_apply_impulse_base(physics_node_base_b, impulse_x, impulse_y, rb_x, rb_y);
        physics_node_a_velocity->x -= physics_node_base_a->inverse_mass * impulse_x;
        physics_node_a_velocity->y -= physics_node_base_a->inverse_mass * impulse_y;

        physics_node_b_velocity->x += physics_node_base_b->inverse_mass * impulse_x;
        physics_node_b_velocity->y += physics_node_base_b->inverse_mass * impulse_y;


        // https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=We%20only%20perform%20positional%20correction%20if%20the%20penetration%20is%20above%20some%20arbitrary%20threshold%2C%20referred%20to%20as%20%22slop%22%3A
        // Do not want the positions to always be shifting due to overlap, add in some slop
        float correction_x = max(contact.collision_normal_penetration - slop, 0.0f) / inv_mass_sum * percent * contact.collision_normal_x;
        float correction_y = max(contact.collision_normal_penetration - slop, 0.0f) / inv_mass_sum * percent * contact.collision_normal_y;

        // Using the normalized collision normal, offset positions of
        // both nodes by the amount they were overlapping (in pixels)
        // when the collision was detected. Split the overlap 50/50
        //
        // Depending on which objects are dynamic, move the dynamic bodies by
        // the penetration amount. Don't want static nodes to be moved by the
        // penetration amount. 
        if(physics_node_a_dynamic == true && physics_node_b_dynamic == false){
            physics_node_base_a->total_position_correction_x += correction_x;
            physics_node_base_a->total_position_correction_y += correction_y;
        }else if(physics_node_a_dynamic == false && physics_node_b_dynamic == true){
            physics_node_base_b->total_position_correction_x -= correction_x;
            physics_node_base_b->total_position_correction_y -= correction_y;
        }else if(physics_node_a_dynamic == true && physics_node_b_dynamic == true){
            physics_node_base_a->total_position_correction_x += correction_x / 2;
             physics_node_base_a->total_position_correction_y += correction_y / 2;

            physics_node_base_b->total_position_correction_x -= correction_x / 2;
            physics_node_base_b->total_position_correction_y -= correction_y / 2;
        }
        // If both were not dynamic, that would have been caught
        // earlier and the collision check would not have happened

        mp_obj_t collision_contact_data[5];
        collision_contact_data[0] = mp_obj_new_float(contact.collision_contact_x);
        collision_contact_data[1] = mp_obj_new_float(contact.collision_contact_y);
        collision_contact_data[2] = mp_obj_new_float(contact.collision_normal_x);
        collision_contact_data[3] = mp_obj_new_float(contact.collision_normal_y);

        mp_obj_t exec[3];

        // Call A callback
        collision_contact_data[4] = node_base_b;
        exec[0] = physics_node_base_a->collision_cb;
        exec[1] = node_base_a->attr_accessor;
        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
        mp_call_method_n_kw(1, 0, exec);

        // Call B callback
        collision_contact_data[4] = node_base_a;
        exec[0] = physics_node_base_b->collision_cb;
        exec[1] = node_base_b->attr_accessor;
        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
        mp_call_method_n_kw(1, 0, exec);
    }
}


void engine_physics_update(float dt){
    // Loop through all nodes and test for collision against
    // all other nodes (not optimized checking of if nodes are
    // even possibly close to each other)
    linked_list_node *physics_link_node_a = engine_physics_nodes.start;
    while(physics_link_node_a != NULL){
        // Now check 'a' against all nodes 'b'
        linked_list_node *physics_link_node_b = engine_physics_nodes.start;

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


void engine_physics_tick(){
    // // https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#timestepping:~:text=Here%20is%20a%20full%20example%3A
    const float alpha = time_accumulator / engine_fps_limit_period_ms;

    // // Apply impulses/move objects due to physics before
    // // checking for collisions. Doing it this way means
    // // you don't see when objects are overlapping and moved
    // // back (looks more stable)
    engine_physics_apply_impulses(engine_fps_limit_period_ms, alpha);

    const float current_time_ms = millis();

    // Store the time elapsed since the last frame began
    time_accumulator += current_time_ms - frame_start_ms;

    // Record the starting of this frame 
    frame_start_ms = current_time_ms;

    // Avoid spiral of death and clamp dt, thus clamping 
    // how many times the update physics can be called in 
    // a single game loop. 
    if(time_accumulator > 30.0f){
        time_accumulator = 30.0f;
    }

    while(time_accumulator > engine_fps_limit_period_ms){
        engine_physics_update(engine_fps_limit_period_ms);
        time_accumulator -= engine_fps_limit_period_ms;
    }
    engine_physics_update(engine_fps_limit_period_ms);
}


linked_list_node *engine_physics_track_node(engine_node_base_t *obj){
    ENGINE_INFO_PRINTF("Tracking physics node %p", obj);
    return linked_list_add_obj(&engine_physics_nodes, obj);
}


void engine_physics_untrack_node(linked_list_node *physics_list_node){
    ENGINE_INFO_PRINTF("Untracking physics node");
    linked_list_del_list_node(&engine_physics_nodes, physics_list_node);
}


void engine_physics_clear_all(){
    ENGINE_INFO_PRINTF("Untracking physics nodes...");
    linked_list_clear(&engine_physics_nodes);
}