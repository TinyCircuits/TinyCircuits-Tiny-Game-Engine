#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2d/physics_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "collision_shapes/polygon_collision_shape_2d.h"
#include "collision_contact_2d.h"

#include <float.h>

linked_list engine_physics_nodes;
float engine_physics_gravity_x = 0.0f;
float engine_physics_gravity_y = -0.00981f;

float engine_physics_fps_limit_period_ms = 16.667f;
float engine_physics_fps_time_at_last_tick_ms = 0.0f;


// https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=A%20helper%20method%20to%20do%20this%20might%20be%3A
void engine_physics_find_min_max_projection(float position_x, float position_y, mp_obj_list_t *vertices_list, float axis_x, float axis_y, float *min, float *max){
    vector2_class_obj_t *vertex = (vector2_class_obj_t*)vertices_list->items[0];
    uint16_t vertex_count = vertices_list->len;

    float projection = engine_math_dot_product(position_x+vertex->x, position_y+vertex->y, axis_x, axis_y);
    *min = projection;
    *max = projection;

    for(uint16_t ivx=1; ivx<vertex_count; ivx++){
        vertex = (vector2_class_obj_t*)vertices_list->items[ivx];
        projection = engine_math_dot_product(position_x+vertex->x, position_y+vertex->y, axis_x, axis_y);
        if(*min > projection){
            *min = projection;
        }

        if(*max < projection){
            *max = projection;
        }
    }
}


bool engine_physics_check_collision(engine_node_base_t *physics_node_base_a, engine_node_base_t *physics_node_base_b, float *collision_normal_x, float *collision_normal_y, float *collision_contact_x, float *collision_contact_y, float *collision_normal_penetration){
    mp_obj_t collision_shape_obj_a = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_collision_shape);
    mp_obj_t collision_shape_obj_b = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_collision_shape);

    if(collision_shape_obj_a != MP_OBJ_NULL && collision_shape_obj_b != MP_OBJ_NULL){
        vector2_class_obj_t *physics_node_a_position = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_position);
        vector2_class_obj_t *physics_node_b_position = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_position);

        if( mp_obj_is_type(collision_shape_obj_a, &polygon_collision_shape_2d_class_type) &&
            mp_obj_is_type(collision_shape_obj_b, &polygon_collision_shape_2d_class_type)){    // Polygon vs. Polygon: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
            polygon_collision_shape_2d_class_obj_t *polygon_shape_a = collision_shape_obj_a;
            polygon_collision_shape_2d_class_obj_t *polygon_shape_b = collision_shape_obj_b;

            mp_obj_list_t *a_normals = polygon_shape_a->normals;
            mp_obj_list_t *b_normals = polygon_shape_b->normals;

            // What if these are children of other nodes? Should this be in absolute? TODO
            float collision_shape_a_pos_x = physics_node_a_position->x;
            float collision_shape_a_pos_y = physics_node_a_position->y;

            float collision_shape_b_pos_x = physics_node_b_position->x;
            float collision_shape_b_pos_y = physics_node_b_position->y;

            float axis_x = 0.0f;
            float axis_y = 0.0f;
            float min0 = 0.0f;
            float max0 = 0.0f;
            float min1 = 0.0f;
            float max1 = 0.0f;
            *collision_normal_penetration = FLT_MAX;

            // https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=it%20would%20look%20like%20something%20like%20this%3A
            for(uint16_t inx=0; inx<a_normals->len; inx++){
                axis_x = ((vector2_class_obj_t*)a_normals->items[inx])->x;
                axis_y = ((vector2_class_obj_t*)a_normals->items[inx])->y;
                engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, polygon_shape_a->vertices, axis_x, axis_y, &min0, &max0);
                engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, polygon_shape_b->vertices, axis_x, axis_y, &min1, &max1);

                if(max0 < min1 || max1 < min0){
                    // No collision
                    return false;
                }else{
                    // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
                    float overlap = fminf(max0, max1) - fmaxf(min0, min1);
                    if(overlap < *collision_normal_penetration){
                        *collision_normal_penetration = overlap;
                        *collision_normal_x = axis_x;
                        *collision_normal_y = axis_y;
                    }
                }
            }
            for(uint16_t inx=0; inx<b_normals->len; inx++){
                axis_x = ((vector2_class_obj_t*)b_normals->items[inx])->x;
                axis_y = ((vector2_class_obj_t*)b_normals->items[inx])->y;
                engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, polygon_shape_a->vertices, axis_x, axis_y, &min0, &max0);
                engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, polygon_shape_b->vertices, axis_x, axis_y, &min1, &max1);

                if(max0 < min1 || max1 < min0){
                    // No collision
                    return false;
                }else{
                    // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
                    float overlap = fminf(max0, max1) - fmaxf(min0, min1);
                    if(overlap < *collision_normal_penetration){
                        *collision_normal_penetration = overlap;
                        *collision_normal_x = axis_x;
                        *collision_normal_y = axis_y;
                    }
                }
            }

            return true;
        }
    }

    return false;
}


void engine_physics_apply_impulses(){
    linked_list_node *physics_link_node = engine_physics_nodes.start;
    while(physics_link_node != NULL){
        engine_node_base_t *physics_node_base = physics_link_node->object;

        bool physics_node_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_dynamic));

        if(physics_node_dynamic){
            vector2_class_obj_t *physics_node_acceleration = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_acceleration);
            vector2_class_obj_t *physics_node_velocity = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_velocity);
            vector2_class_obj_t *physics_node_position = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_position);
            vector2_class_obj_t *physics_node_gravity_scale = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_gravity_scale);

            // Modifying these directly is good enough, don't need mp_store_attr even if using classes at main level!
            // Apply the user defined acceleration
            physics_node_velocity->x += physics_node_acceleration->x;
            physics_node_velocity->y += physics_node_acceleration->y;

            // Apply engine gravity (can be modifed by the user)
            physics_node_velocity->x -= engine_physics_gravity_x * physics_node_gravity_scale->x;
            physics_node_velocity->y -= engine_physics_gravity_y * physics_node_gravity_scale->y;

            // Apply velocity to the position
            physics_node_position->x += physics_node_velocity->x;
            physics_node_position->y += physics_node_velocity->y;
        }

        physics_link_node = physics_link_node->next;
    }
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
            if(physics_link_node_a->object != physics_link_node_b->object){
                engine_node_base_t *physics_node_base_a = physics_link_node_a->object;
                engine_node_base_t *physics_node_base_b = physics_link_node_b->object;

                // Could use these flags to stop some calculations below: TODO
                bool physics_node_a_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_dynamic));
                bool physics_node_b_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_dynamic));

                // Only check collision if at atleast one of the involved nodes
                // is dynamic (do not want static nodes generating collisions)
                if(physics_node_a_dynamic == true || physics_node_b_dynamic == true){
                    // If a none null collision point is received that means
                    // a collision occurred. Call user callback and adjust
                    // physics node's velocity
                    float collision_normal_x = 0.0f;
                    float collision_normal_y = 0.0f;
                    float collision_contact_x = 0.0f;
                    float collision_contact_y = 0.0f;
                    float collision_normal_penetration = 0.0f;

                    if(engine_physics_check_collision(physics_node_base_a, physics_node_base_b, &collision_normal_x, &collision_normal_y, &collision_contact_x, &collision_contact_y, &collision_normal_penetration)){
                        ENGINE_FORCE_PRINTF("COLLISION: %0.3f, %0.3f, %0.3f", collision_normal_x, collision_normal_y, collision_normal_penetration);

                        engine_node_base_t *node_base;
                        engine_physics_2d_node_common_data_t *physics_2d_node_common_data;

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

                        // Using the normalized collision normal, offset positions of
                        // both nodes by the amount they were overlapping (in pixels)
                        // when the collision was detected. Split the overlap 50/50
                        vector2_class_obj_t *physics_node_a_position = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_position);
                        vector2_class_obj_t *physics_node_b_position = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_position);

                        // Depending on which objects are dynamic, save data. Don't want static nodes
                        // to be moved by the penetration amount. This will likely mean that the one
                        // dynamic node will be moved by half and still be colliding. The next time
                        // it is found to be colliding means it should move some more. This will
                        // result in lots of collision callbacks, should move the dynamic object the
                        // full amount if there's a static object: TODO
                        // NOTE: DO NOT NEED TO DO mp_store_attr since we're modifying using the pointers!
                        if(physics_node_a_dynamic){
                            physics_node_a_position->x -= collision_normal_x * collision_normal_penetration / 2;
                            physics_node_a_position->y -= collision_normal_y * collision_normal_penetration / 2;
                        }

                        if(physics_node_b_dynamic){
                            physics_node_b_position->x += collision_normal_x * collision_normal_penetration / 2;
                            physics_node_b_position->y += collision_normal_y * collision_normal_penetration / 2;
                        }

                        mp_obj_t collision_contact_data[5];
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
            }
            physics_link_node_b = physics_link_node_b->next;
        }
        physics_link_node_a = physics_link_node_a->next;
    }

    ENGINE_FORCE_PRINTF(" ");
    engine_physics_apply_impulses();
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