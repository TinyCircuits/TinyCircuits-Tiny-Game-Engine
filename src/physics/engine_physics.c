#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2D/physics_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "nodes/2D/physics_2d_node.h"
#include "collision_shapes/circle_collision_shape_2d.h"
#include "collision_contact_2d.h"

linked_list engine_physics_nodes;
float gravity_x = 0.0f;
float gravity_y = -0.00981f;

float engine_physics_fps_limit_period_ms = 33.333f;
float engine_physics_fps_time_at_last_tick_ms = 0.0f;


bool engine_physics_check_collision(engine_node_base_t *physics_node_base_a, engine_node_base_t *physics_node_base_b, float *collision_normal_x, float *collision_normal_y, float *collision_contact_x, float *collision_contact_y){
    mp_obj_t collision_shape_obj_a = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_collision_shape);
    mp_obj_t collision_shape_obj_b = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_collision_shape);

    if(collision_shape_obj_a != MP_OBJ_NULL && collision_shape_obj_b != MP_OBJ_NULL){
        vector2_class_obj_t *physics_node_a_position = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_position);
        vector2_class_obj_t *physics_node_b_position = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_position);

        if(mp_obj_is_type(collision_shape_obj_a, &circle_collision_shape_2d_class_type) &&
           mp_obj_is_type(collision_shape_obj_b, &circle_collision_shape_2d_class_type)){    // Circle vs Circle: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=must%20be%20extended.-,Circle%20vs%20Circle,-Lets%20start%20with

            circle_collision_shape_2d_class_obj_t *collision_shape_a = collision_shape_obj_a;
            circle_collision_shape_2d_class_obj_t *collision_shape_b = collision_shape_obj_b;

            float collision_shape_a_pos_x = collision_shape_a->position->x + physics_node_a_position->x;
            float collision_shape_a_pos_y = collision_shape_a->position->y + physics_node_a_position->y;

            float collision_shape_b_pos_x = collision_shape_b->position->x + physics_node_b_position->x;
            float collision_shape_b_pos_y = collision_shape_b->position->y + physics_node_b_position->y;

            // Normal vector from A to B
            float normal_x = collision_shape_b_pos_x - collision_shape_a_pos_x;
            float normal_y = collision_shape_b_pos_y - collision_shape_a_pos_y;

            // Distances without sqrts
            float normal_length_squared = (normal_x*normal_x) + (normal_y*normal_y);
            float distance_between_squared = collision_shape_a->radius + collision_shape_b->radius;
            distance_between_squared *= distance_between_squared;

            // If true, not colliding and should stop here
            if(normal_length_squared > distance_between_squared){
                return false;
            }

            // Circles are colliding, compute information for CollisionContact2D
            float distance_between = sqrt(distance_between_squared);
            *collision_normal_x = 1.0f;
            *collision_normal_y = 0.0f;

            *collision_contact_x = collision_shape_a_pos_x;
            *collision_contact_y = collision_shape_a_pos_y;

            // If not exactly in the same position, compute position and normal.
            // Otherwise if the positions are the same, provide easy to get and
            // default information as set above
            if(distance_between != 0.0f){
                *collision_normal_x = normal_x / distance_between;
                *collision_normal_y = normal_y / distance_between;

                *collision_contact_x = *collision_normal_x * collision_shape_a->radius + collision_shape_a_pos_x;
                *collision_contact_y = *collision_normal_y * collision_shape_a->radius + collision_shape_a_pos_y;
            }

            return true;
        }
    }

    return false;
}


void engine_physics_tick(){
    // If it is not time to update physics, then don't, otherwise track when physics was updated last
    if(millis() - engine_physics_fps_time_at_last_tick_ms < engine_physics_fps_limit_period_ms){
        return;
    }else{
        engine_physics_fps_time_at_last_tick_ms = millis();
    }

    // Loop through all nodes and test for collision against
    // all other nodes (not optimized checking of if nodes are
    // even possibly close to each other)
    linked_list_node *physics_link_node_a = engine_physics_nodes.start;
    while(physics_link_node_a != NULL){
        // Now check 'a' against all nodes 'b'
        linked_list_node *physics_link_node_b = engine_physics_nodes.start;

        while(physics_link_node_b != NULL){
            // Make sure we are not checking against ourselves
            if(physics_link_node_a != physics_link_node_b){

                mp_obj_t collision_contact_data[5];
                engine_node_base_t *physics_node_base_a = physics_link_node_a->object;
                engine_node_base_t *physics_node_base_b = physics_link_node_b->object;

                // If a none null collision point is received that means
                // a collision occurred. Call user callback and adjust
                // physics node's velocity
                float collision_normal_x = 0.0f;
                float collision_normal_y = 0.0f;
                float collision_contact_x = 0.0f;
                float collision_contact_y = 0.0f;

                if(engine_physics_check_collision(physics_node_base_a, physics_node_base_b, &collision_normal_x, &collision_normal_y, &collision_contact_x, &collision_contact_y)){
                    engine_node_base_t *node_base;
                    engine_physics_2d_node_common_data_t *physics_2d_node_common_data;

                    // Could use these flags to stop some calculations below: TODO
                    bool physics_node_a_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_dynamic));
                    bool physics_node_b_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_dynamic));

                    // Resolve collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
                    vector2_class_obj_t *physics_node_a_velocity = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_velocity);
                    vector2_class_obj_t *physics_node_b_velocity = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_velocity);

                    // If either node is not dynamic, set any velocities to zero no matter what set to
                    if(!physics_node_a_dynamic){
                        physics_node_a_velocity->x = 0.0f;
                        physics_node_a_velocity->y = 0.0f;
                    }

                    if(!physics_node_b_dynamic){
                        physics_node_b_velocity->x = 0.0f;
                        physics_node_b_velocity->y = 0.0f;
                    }

                    float relative_velocity_x = physics_node_b_velocity->x - physics_node_a_velocity->x;
                    float relative_velocity_y = physics_node_b_velocity->y - physics_node_a_velocity->y;

                    float velocity_along_collision_normal = engine_math_dot_product(relative_velocity_x, relative_velocity_y, collision_normal_x, collision_normal_y);

                    // Do not resolve if velocities are separating
                    if(velocity_along_collision_normal > 0){
                        physics_link_node_b = physics_link_node_b->next;
                        continue;
                    }

                    // Calculate restitution/bounciness
                    float physics_node_a_bounciness = mp_obj_get_float(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_bounciness));
                    float physics_node_b_bounciness = mp_obj_get_float(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_bounciness));
                    float physics_node_a_mass = mp_obj_get_float(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_mass));
                    float physics_node_b_mass = mp_obj_get_float(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_mass));
                    float bounciness = fminf(physics_node_a_bounciness, physics_node_b_bounciness);

                    float physics_node_a_inverse_mass = 0.0f;
                    float physics_node_b_inverse_mass = 0.0f;

                    // Avoid divide by zero for inverse mass calculation
                    if(physics_node_a_mass == 0.0f){
                        physics_node_a_inverse_mass = 0.0f; 
                    }else{
                        physics_node_a_inverse_mass = 1.0f / physics_node_a_mass;
                    }

                    if(physics_node_b_mass == 0.0f){
                        physics_node_b_inverse_mass = 0.0f; 
                    }else{
                        physics_node_b_inverse_mass = 1.0f / physics_node_b_mass;
                    }

                    float impulse_coefficient_j = -(1 + bounciness) * velocity_along_collision_normal;
                    impulse_coefficient_j /= physics_node_a_inverse_mass + physics_node_b_inverse_mass;

                    float impulse_x = impulse_coefficient_j * collision_normal_x;
                    float impulse_y = impulse_coefficient_j * collision_normal_y;

                    physics_node_a_velocity->x -= 1 / physics_node_a_mass * impulse_x;
                    physics_node_a_velocity->y -= 1 / physics_node_a_mass * impulse_y;

                    physics_node_b_velocity->x += 1 / physics_node_b_mass * impulse_x;
                    physics_node_b_velocity->y += 1 / physics_node_b_mass * impulse_y;

                    if(physics_node_a_dynamic) mp_store_attr(physics_node_base_a->attr_accessor, MP_QSTR_velocity, physics_node_a_velocity);
                    if(physics_node_b_dynamic) mp_store_attr(physics_node_base_b->attr_accessor, MP_QSTR_velocity, physics_node_b_velocity);

                    collision_contact_data[0] = mp_obj_new_float(collision_contact_x);
                    collision_contact_data[1] = mp_obj_new_float(collision_contact_y);
                    collision_contact_data[2] = mp_obj_new_float(collision_normal_x);
                    collision_contact_data[3] = mp_obj_new_float(collision_normal_y);

                    mp_obj_t exec[3];

                    // Call A callback
                    node_base = physics_link_node_a->object;
                    physics_2d_node_common_data = node_base->node_common_data;
                    collision_contact_data[4] = physics_link_node_b->object;
                    exec[0] = physics_2d_node_common_data->collision_cb;
                    exec[1] = node_base->attr_accessor;
                    exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                    mp_call_method_n_kw(1, 0, exec);

                    // Call B callback
                    node_base = physics_link_node_b->object;
                    physics_2d_node_common_data = node_base->node_common_data;
                    collision_contact_data[4] = physics_link_node_a->object;
                    exec[0] = physics_2d_node_common_data->collision_cb;
                    exec[1] = node_base->attr_accessor;
                    exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                    mp_call_method_n_kw(1, 0, exec);
                }
            }
            physics_link_node_b = physics_link_node_b->next;
        }
        physics_link_node_a = physics_link_node_a->next;
    }


    linked_list_node *physics_link_node = engine_physics_nodes.start;
    while(physics_link_node != NULL){
        engine_node_base_t *physics_node_base = physics_link_node->object;

        bool physics_node_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_dynamic));

        if(physics_node_dynamic){
            vector2_class_obj_t *physics_node_velocity = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_velocity);
            physics_node_velocity->x -= gravity_x;
            physics_node_velocity->y -= gravity_y;

            vector2_class_obj_t *physics_node_position = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_position);

            physics_node_position->x += physics_node_velocity->x;
            physics_node_position->y += physics_node_velocity->y;

            mp_store_attr(physics_node_base->attr_accessor, MP_QSTR_velocity, physics_node_velocity);
            mp_store_attr(physics_node_base->attr_accessor, MP_QSTR_position, physics_node_position);
        }

        physics_link_node = physics_link_node->next;
    }


    // // Update all node positions
    // linked_list_node *current_physics_node = engine_physics_nodes.start;
    // while(current_physics_node != NULL){

    //     engine_node_base_t *node_base = current_physics_node->object;
        
    //     vector2_class_obj_t *vel = mp_load_attr(node_base->attr_accessor, MP_QSTR_velocity);
    //     vector2_class_obj_t *pos = mp_load_attr(node_base->attr_accessor, MP_QSTR_position);
    //     mp_float_t rotation = mp_obj_get_float(mp_load_attr(node_base->attr_accessor, MP_QSTR_rotation));
    //     mp_float_t angular_vel = mp_obj_get_float(mp_load_attr(node_base->attr_accessor, MP_QSTR_angular_velocity));
    //     vector2_class_obj_t *acceleration = mp_load_attr(node_base->attr_accessor, MP_QSTR_acceleration);

    //     pos->x += vel->x;
    //     pos->y += vel->y;

    //     vel->y += gravity+acceleration->y;

    //     mp_store_attr(node_base->attr_accessor, MP_QSTR_rotation, mp_obj_new_float(rotation+angular_vel));
    //     mp_store_attr(node_base->attr_accessor, MP_QSTR_angular_velocity, mp_obj_new_float(0));

    //     current_physics_node = current_physics_node->next;
    // }

    // ENGINE_INFO_PRINTF("Clearing manifolds, list length is %d\n\r", engine_physics_manifolds.count);
    // // Clear out the manifolds
    // linked_list_node *node_m = engine_physics_manifolds.start;
    // int manifolds = 0;
    // while(node_m != NULL) {
    //     linked_list_node *next = node_m->next;
    //     linked_list_del_list_node(&engine_physics_manifolds, node_m);
    //     ENGINE_INFO_PRINTF("Deleted manifold\n\r\n\r");
    //     node_m = next;
    //     manifolds++;
    // }

    // ENGINE_INFO_PRINTF("Cleared %f manifolds, Generating manifolds...\n\r", (float)manifolds);
    // manifolds = 0;
    // // Generate manifolds
    // ENGINE_INFO_PRINTF("Physics node list length is %d\n\r", engine_physics_nodes.count);
    // linked_list_node *node_a = engine_physics_nodes.start;
    // while(node_a != NULL){

    //     //engine_node_base_t *a_physics_node_base = node_a->object;
    //     linked_list_node *node_b = node_a->next;

    //     while(node_b != NULL){

    //         mp_obj_t m = physics_2d_node_class_test(node_a->object, node_b->object);

    //         ((physics_manifold_class_obj_t*)MP_OBJ_TO_PTR(m))->body_a = node_a->object;
    //         ((physics_manifold_class_obj_t*)MP_OBJ_TO_PTR(m))->body_b = node_b->object;

    //         linked_list_add_obj(&engine_physics_manifolds, MP_OBJ_TO_PTR(m));
    //         manifolds++;

    //         node_b = node_b->next;
    //     }
    //     node_a = node_a->next;
    // }

    // ENGINE_INFO_PRINTF("Generated %f manifolds\n\r", (float)manifolds);

    // ENGINE_INFO_PRINTF("Applying manifolds\n\r");
    // // Apply manifold impulses
    // node_m = engine_physics_manifolds.start;
    // while(node_m != NULL) {
    //     physics_manifold_class_obj_t* m = MP_OBJ_TO_PTR(node_m->object);
    //     ENGINE_INFO_PRINTF("Applying manifold %p\n\r", m);
    //     physics_2d_node_class_apply_manifold_impulse(m->body_a, m->body_b, MP_OBJ_FROM_PTR(m));
    //     node_m = node_m->next;
    // }
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