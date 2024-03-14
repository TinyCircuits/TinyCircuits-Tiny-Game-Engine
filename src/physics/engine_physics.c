#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2d/physics_rectangle_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "utility/engine_bit_collection.h"
#include "collision_contact_2d.h"
#include "nodes/node_types.h"
#include "py/obj.h"
#include "draw/engine_display_draw.h"

#include <float.h>

// The maximum number of physics nodes that can exist in a scene is restricted
// by the number of IDs available to give those nodes. The number of IDs is
// limited so that IDs can be used with a simple paring function to index in
// a large bit array to check if collisions between objects have already occurred
#define PHYSICS_ID_MAX 180

// Bit array/collection to track nodes that have collided. In the `init` function
// this is sized so that the output indices from a simple paring function can fit
// (https://math.stackexchange.com/a/531914)
engine_bit_collection_t collided_physics_nodes;

// Buffer of available physics IDs. Filled with values
// 1 -> `PHYSICS_ID_MAX` on physics `init`. 
uint8_t available_physics_ids[PHYSICS_ID_MAX];
uint8_t next_available_physics_id = 0;


// Fill `available_physics_ids` with IDs from `1` -> `PHYSICS_ID_MAX`
// through indices `0` -> `PHYSICS_ID_MAX-1`
void engine_physics_init_ids(){
    for(uint8_t idx=0; idx<PHYSICS_ID_MAX; idx++){
        available_physics_ids[idx] = idx+1;
    }
}


uint8_t engine_physics_take_available_id(){
    if(next_available_physics_id >= PHYSICS_ID_MAX){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Ran out of IDs to give physics nodes..."));
    }

    uint8_t id = available_physics_ids[next_available_physics_id];
    next_available_physics_id++;
    return id;
}


void engine_physics_give_back_id(uint8_t id){
    next_available_physics_id--;
    available_physics_ids[next_available_physics_id] = id;
}


// Using a simple pairing function (https://math.stackexchange.com/a/531914)
// calculate a unique number for the input IDs
uint32_t engine_physics_get_pair_index(uint8_t a_id, uint8_t b_id){
    return PHYSICS_ID_MAX * (uint32_t)a_id + (uint32_t)b_id;
}




linked_list engine_physics_nodes;
float engine_physics_gravity_x = 0.0f;
float engine_physics_gravity_y = -0.00981f;

float engine_physics_fps_limit_period_ms = 16.667f;
float engine_physics_fps_time_at_last_tick_ms = 0.0f;


// https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=A%20helper%20method%20to%20do%20this%20might%20be%3A
void engine_physics_find_min_max_projection(float position_x, float position_y, float *vertices_x, float *vertices_y, float axis_x, float axis_y, float *min, float *max){
    float projection = engine_math_dot_product(position_x+vertices_x[0], position_y+vertices_y[0], axis_x, axis_y);
    *min = projection;
    *max = projection;

    for(uint16_t ivx=1; ivx<4; ivx++){
        projection = engine_math_dot_product(position_x+vertices_x[ivx], position_y+vertices_y[ivx], axis_x, axis_y);
        if(*min > projection){
            *min = projection;
        }

        if(*max < projection){
            *max = projection;
        }
    }
}


void engine_physics_rect_rect_get_best(float px, float py, float collision_normal_x, float collision_normal_y, float *max_proj_vertex_x, float *max_proj_vertex_y, float *edge_v0_x, float *edge_v0_y, float *edge_v1_x, float *edge_v1_y, float *vertices_x, float *vertices_y){
    float max = -FLT_MAX;
    uint8_t vertex_index = 0;

    // Step 1: Find farthest vertex in polygon along separation normal
    for(uint8_t ivx=0; ivx<4; ivx++){
        float projection = engine_math_dot_product(collision_normal_x, collision_normal_y, px+vertices_x[ivx], py+vertices_y[ivx]);
        if(projection > max){
            max = projection;
            vertex_index = ivx;
        }
    }

    // Step 2: Find most perpendicular vertex
    *max_proj_vertex_x = px+vertices_x[vertex_index];
    *max_proj_vertex_y = py+vertices_y[vertex_index];

    float v1x = 0.0f;
    float v1y = 0.0f;

    float v0x = 0.0f;
    float v0y = 0.0f;

    if(vertex_index + 1 > 3){
        v1x = px+vertices_x[0];
        v1y = py+vertices_y[0];
    }else{
        v1x = px+vertices_x[vertex_index+1];
        v1y = py+vertices_y[vertex_index+1];
    }

    if(vertex_index - 1 < 0){
        v0x = px+vertices_x[3];
        v0y = py+vertices_y[3];
    }else{
        v0x = px+vertices_x[vertex_index-1];
        v0y = py+vertices_y[vertex_index-1];
    }

    float left_v_x = *max_proj_vertex_x - v1x;
    float left_v_y = *max_proj_vertex_y - v1y;

    float right_v_x = *max_proj_vertex_x - v0x;
    float right_v_y = *max_proj_vertex_y - v0y;

    engine_math_normalize(&left_v_x, &left_v_y);
    engine_math_normalize(&right_v_x, &right_v_y);

    if(engine_math_dot_product(right_v_x, right_v_y, collision_normal_x, collision_normal_y) <= engine_math_dot_product(left_v_x, left_v_y, collision_normal_x, collision_normal_y)){
        *edge_v0_x = v0x;
        *edge_v0_y = v0y;
        *edge_v1_x = *max_proj_vertex_x;
        *edge_v1_y = *max_proj_vertex_y;
    }else{
        *edge_v0_x = *max_proj_vertex_x;
        *edge_v0_y = *max_proj_vertex_y;
        *edge_v1_x = v1x;
        *edge_v1_y = v1y;
    }
}


// Get the contact point from the OBB to OBB collision: https://dyn4j.org/2011/11/contact-points-using-clipping/
void engine_physics_rect_rect_get_contact(float collision_normal_x, float collision_normal_y, float *collision_contact_x, float *collision_contact_y, engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_a, engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_b){
    float a_max_proj_vertex_x = 0.0f;
    float a_max_proj_vertex_y = 0.0f;

    float a_edge_v0_x = 0.0f;
    float a_edge_v0_y = 0.0f;
    float a_edge_v1_x = 0.0f;
    float a_edge_v1_y = 0.0f;

    float b_max_proj_vertex_x = 0.0f;
    float b_max_proj_vertex_y = 0.0f;

    float b_edge_v0_x = 0.0f;
    float b_edge_v0_y = 0.0f;
    float b_edge_v1_x = 0.0f;
    float b_edge_v1_y = 0.0f;

    float apx = ((vector2_class_obj_t*)physics_rectangle_a->position)->x;
    float apy = ((vector2_class_obj_t*)physics_rectangle_a->position)->y;

    float bpx = ((vector2_class_obj_t*)physics_rectangle_b->position)->x;
    float bpy = ((vector2_class_obj_t*)physics_rectangle_b->position)->y;
    
    engine_physics_rect_rect_get_best(apx, apy, -collision_normal_x, -collision_normal_y, &a_max_proj_vertex_x, &a_max_proj_vertex_y, &a_edge_v0_x, &a_edge_v0_y, &a_edge_v1_x, &a_edge_v1_y, physics_rectangle_a->vertices_x, physics_rectangle_a->vertices_y);
    engine_physics_rect_rect_get_best(bpx, bpy,  collision_normal_x,  collision_normal_y, &b_max_proj_vertex_x, &b_max_proj_vertex_y, &b_edge_v0_x, &b_edge_v0_y, &b_edge_v1_x, &b_edge_v1_y, physics_rectangle_b->vertices_x, physics_rectangle_b->vertices_y);

    // Already know that the intersection will happen, just need to handle case
    // where the two edges can be exactly parallel. Project edges onto this parallel
    // line, find the smallest portion, get the midpoint, use that as the intersection
    if(engine_math_2d_do_segments_intersect(a_edge_v0_x, a_edge_v0_y, a_edge_v1_x, a_edge_v1_y, b_edge_v0_x, b_edge_v0_y, b_edge_v1_x, b_edge_v1_y, collision_contact_x, collision_contact_y) == false){
        float to_project_x[] = {a_edge_v0_x, a_edge_v1_x, b_edge_v0_x, b_edge_v1_x};
        float to_project_y[] = {a_edge_v0_y, a_edge_v1_y, b_edge_v0_y, b_edge_v1_y};

        // Make this project vector perpendicular
        float projections[4];
        projections[0] = engine_math_dot_product(a_edge_v0_x, a_edge_v0_y, collision_normal_y, -collision_normal_x);
        projections[1] = engine_math_dot_product(a_edge_v1_x, a_edge_v1_y, collision_normal_y, -collision_normal_x);
        projections[2] = engine_math_dot_product(b_edge_v0_x, b_edge_v0_y, collision_normal_y, -collision_normal_x);
        projections[3] = engine_math_dot_product(b_edge_v1_x, b_edge_v1_y, collision_normal_y, -collision_normal_x);

        // https://math.stackexchange.com/a/2196332
        for(uint8_t i=0; i<4; ++i){
            for(uint8_t j=i+1; j<4; ++j){
                if(projections[i] > projections[j]){
                    engine_math_swap(&projections[i], &projections[j]);
                    engine_math_swap(&to_project_x[i], &to_project_x[j]);
                    engine_math_swap(&to_project_y[i], &to_project_y[j]);
                }
            }
        }

        // The contact point of two parallel lines is the midpoint of the overlapping area
        engine_math_2d_midpoint(to_project_x[1], to_project_y[1], to_project_x[2], to_project_y[2], collision_contact_x, collision_contact_y);
    }
}


// rectangle vs. rectangle: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
bool engine_physics_check_rect_rect_collision(engine_node_base_t *physics_node_base_a, engine_node_base_t *physics_node_base_b, float *collision_normal_x, float *collision_normal_y, float *collision_contact_x, float *collision_contact_y, float *collision_normal_penetration){
    engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_a = physics_node_base_a->node;
    engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_b = physics_node_base_b->node;

    vector2_class_obj_t *physics_rectangle_a_position = physics_rectangle_a->position;
    vector2_class_obj_t *physics_rectangle_b_position = physics_rectangle_b->position;

    // What if these are children of other nodes? Should this be in absolute? TODO
    float collision_shape_a_pos_x = physics_rectangle_a_position->x;
    float collision_shape_a_pos_y = physics_rectangle_a_position->y;

    float collision_shape_b_pos_x = physics_rectangle_b_position->x;
    float collision_shape_b_pos_y = physics_rectangle_b_position->y;

    float axis_x = 0.0f;
    float axis_y = 0.0f;
    float a_min = 0.0f;
    float a_max = 0.0f;
    float b_min = 0.0f;
    float b_max = 0.0f;
    *collision_normal_penetration = FLT_MAX;

    // https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=it%20would%20look%20like%20something%20like%20this%3A
    // Only need to test two axes since rectangles have parallel directions: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/2d-rotated-rectangle-collision-r2604/#:~:text=This%20gives%20us-,four%20axes,-%2C%20each%20of%20which
    for(uint8_t inx=0; inx<2; inx++){
        axis_x = physics_rectangle_a->normals_x[inx];
        axis_y = physics_rectangle_a->normals_y[inx];
        engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, physics_rectangle_a->vertices_x, physics_rectangle_a->vertices_y, axis_x, axis_y, &a_min, &a_max);
        engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, physics_rectangle_b->vertices_x, physics_rectangle_b->vertices_y, axis_x, axis_y, &b_min, &b_max);

        if(a_max < b_min || b_max < a_min){
            // No collision
            return false;
        }else{
            // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
            float overlap = fminf(a_max, b_max) - fmaxf(a_min, b_min);
            if(overlap < *collision_normal_penetration){
                *collision_normal_penetration = overlap;
                *collision_normal_x = axis_x;
                *collision_normal_y = axis_y;
            }
        }
    }
    for(uint8_t inx=0; inx<2; inx++){
        axis_x = physics_rectangle_b->normals_x[inx];
        axis_y = physics_rectangle_b->normals_y[inx];
        engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, physics_rectangle_a->vertices_x, physics_rectangle_a->vertices_y, axis_x, axis_y, &a_min, &a_max);
        engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, physics_rectangle_b->vertices_x, physics_rectangle_b->vertices_y, axis_x, axis_y, &b_min, &b_max);

        if(a_max < b_min || b_max < a_min){
            // No collision
            return false;
        }else{
            // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
            float overlap = fminf(a_max, b_max) - fmaxf(a_min, b_min);
            if(overlap < *collision_normal_penetration){
                *collision_normal_penetration = overlap;
                *collision_normal_x = axis_x;
                *collision_normal_y = axis_y;
            }
        }
    }

    return true;
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


void engine_physics_init(){
    ENGINE_INFO_PRINTF("EnginePhysics: Starting...")
    engine_physics_init_ids();
    engine_bit_collection_create(&collided_physics_nodes, engine_physics_get_pair_index(PHYSICS_ID_MAX, PHYSICS_ID_MAX));
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
                bool (*check_collision)(void*, void*, float*, float*, float*, float*, float*);

                bool physics_node_a_dynamic = false;
                bool physics_node_b_dynamic = false;

                uint8_t physics_node_a_id = 0;
                uint8_t physics_node_b_id = 0;

                vector2_class_obj_t *physics_node_a_position = NULL;
                vector2_class_obj_t *physics_node_b_position = NULL;

                vector2_class_obj_t *physics_node_a_velocity = NULL;
                vector2_class_obj_t *physics_node_b_velocity = NULL;

                mp_obj_t collision_cb_a = MP_OBJ_NULL;
                mp_obj_t collision_cb_b = MP_OBJ_NULL;

                if(physics_node_base_a->type == NODE_TYPE_PHYSICS_RECTANGLE_2D && physics_node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){
                    engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_a = physics_node_base_a->node;
                    engine_physics_rectangle_2d_node_class_obj_t *physics_rectangle_b = physics_node_base_b->node;

                    physics_node_a_dynamic = mp_obj_get_int(physics_rectangle_a->dynamic);
                    physics_node_b_dynamic = mp_obj_get_int(physics_rectangle_b->dynamic);

                    physics_node_a_id = physics_rectangle_a->physics_id;
                    physics_node_b_id = physics_rectangle_b->physics_id;

                    physics_node_a_position = physics_rectangle_a->position;
                    physics_node_b_position = physics_rectangle_b->position;

                    physics_node_a_velocity = physics_rectangle_a->velocity;
                    physics_node_b_velocity = physics_rectangle_b->velocity;

                    check_collision = engine_physics_check_rect_rect_collision;

                    collision_cb_a = physics_rectangle_a->collision_cb;
                    collision_cb_b = physics_rectangle_b->collision_cb;
                }else if(physics_node_base_a->type == NODE_TYPE_PHYSICS_RECTANGLE_2D && physics_node_base_b->type == NODE_TYPE_PHYSICS_CIRCLE_2D){

                }else if(physics_node_base_a->type == NODE_TYPE_PHYSICS_CIRCLE_2D && physics_node_base_b->type == NODE_TYPE_PHYSICS_RECTANGLE_2D){

                }else if(physics_node_base_a->type == NODE_TYPE_PHYSICS_CIRCLE_2D && physics_node_base_b->type == NODE_TYPE_PHYSICS_CIRCLE_2D){

                }

                // Only check collision if at atleast one of the involved nodes
                // is dynamic (do not want static nodes generating collisions)
                if(physics_node_a_dynamic == true || physics_node_b_dynamic == true){

                    // To consistently generate the index for `already_collided`
                    // the ID pair needs to be sorted consistently (don't matter
                    // how, just as long as it is consistent)
                    uint32_t pair_index = 0;

                    // Sort so that id_max*a_id + b_id comes out the same
                    // no matter which node is `a` or `b`
                    // // Get the perfect hash code (can be perfect since the range is limited)
                    if(physics_node_a_id > physics_node_b_id){
                        pair_index = engine_physics_get_pair_index(physics_node_b_id, physics_node_a_id);
                    }else{
                        pair_index = engine_physics_get_pair_index(physics_node_a_id, physics_node_b_id);
                    }
                    
                    bool already_collided = engine_bit_collection_get(&collided_physics_nodes, pair_index);

                    // Skip collision check if already collied
                    // before, otherwise set that these have collied
                    // this frame already
                    if(already_collided){
                        physics_link_node_b = physics_link_node_b->next;
                        continue;
                    }else{
                        engine_bit_collection_set(&collided_physics_nodes, pair_index);
                    }

                    float collision_normal_x = 0.0f;
                    float collision_normal_y = 0.0f;
                    float collision_contact_x = 0.0f;
                    float collision_contact_y = 0.0f;
                    float collision_normal_penetration = 0.0f;

                    if(check_collision(physics_node_base_a, physics_node_base_b, &collision_normal_x, &collision_normal_y, &collision_contact_x, &collision_contact_y, &collision_normal_penetration)){
                        // `collision_normal_x` and `collision_normal_y` will point in any direction,
                        // need to discern if the normal axis should be flipped so that the objects
                        // move away from each other: https://stackoverflow.com/a/6244218
                        float a_to_b_direction_x = physics_node_b_position->x - physics_node_a_position->x;
                        float a_to_b_direction_y = physics_node_b_position->y - physics_node_a_position->y;

                        if(engine_math_dot_product(collision_normal_x, collision_normal_y, a_to_b_direction_x, a_to_b_direction_y) >= 0.0f){
                            collision_normal_x = -collision_normal_x;
                            collision_normal_y = -collision_normal_y;
                        }

                        // Resolve collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
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

                        // Do not resolve if velocities are separating (this does mean
                        // objects inside each other will not collide until a non separating
                        // velocity is set)
                        if(velocity_along_collision_normal <= 0.0f){
                            physics_link_node_b = physics_link_node_b->next;
                            continue;
                        }

                        engine_physics_rect_rect_get_contact(collision_normal_x, collision_normal_y, &collision_contact_x, &collision_contact_y, physics_node_base_a->node, physics_node_base_b->node);

                        // Calculate restitution/bounciness
                        float physics_node_a_bounciness = mp_obj_get_float(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_bounciness));
                        float physics_node_b_bounciness = mp_obj_get_float(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_bounciness));
                        float physics_node_a_mass = mp_obj_get_float(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_mass));
                        float physics_node_b_mass = mp_obj_get_float(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_mass));
                        float bounciness = fminf(physics_node_a_bounciness, physics_node_b_bounciness); // Restitution

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

                        physics_node_a_velocity->x -= physics_node_a_inverse_mass * impulse_x;
                        physics_node_a_velocity->y -= physics_node_a_inverse_mass * impulse_y;

                        physics_node_b_velocity->x += physics_node_b_inverse_mass * impulse_x;
                        physics_node_b_velocity->y += physics_node_b_inverse_mass * impulse_y;

                        // Using the normalized collision normal, offset positions of
                        // both nodes by the amount they were overlapping (in pixels)
                        // when the collision was detected. Split the overlap 50/50
                        //
                        // Depending on which objects are dynamic, move the dynamic bodies by
                        // the penetration amount. Don't want static nodes to be moved by the
                        // penetration amount. 
                        if(physics_node_a_dynamic == true && physics_node_b_dynamic == false){
                            physics_node_a_position->x += collision_normal_x * collision_normal_penetration;
                            physics_node_a_position->y += collision_normal_y * collision_normal_penetration;
                        }else if(physics_node_a_dynamic == false && physics_node_b_dynamic == true){
                            physics_node_b_position->x -= collision_normal_x * collision_normal_penetration;
                            physics_node_b_position->y -= collision_normal_y * collision_normal_penetration;
                        }else if(physics_node_a_dynamic == true && physics_node_b_dynamic == true){
                            physics_node_a_position->x += collision_normal_x * collision_normal_penetration / 2;
                            physics_node_a_position->y += collision_normal_y * collision_normal_penetration / 2;

                            physics_node_b_position->x -= collision_normal_x * collision_normal_penetration / 2;
                            physics_node_b_position->y -= collision_normal_y * collision_normal_penetration / 2;
                        }
                        // If both were not dynamic, that would have been caught
                        // earlier and the collision check would not have happened


                        mp_obj_t collision_contact_data[5];
                        collision_contact_data[0] = mp_obj_new_float(collision_contact_x);
                        collision_contact_data[1] = mp_obj_new_float(collision_contact_y);
                        collision_contact_data[2] = mp_obj_new_float(collision_normal_x);
                        collision_contact_data[3] = mp_obj_new_float(collision_normal_y);

                        mp_obj_t exec[3];

                        // Call A callback
                        collision_contact_data[4] = physics_link_node_b->object;
                        exec[0] = collision_cb_a;
                        exec[1] = physics_node_base_a->attr_accessor;
                        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                        mp_call_method_n_kw(1, 0, exec);

                        // Call B callback
                        collision_contact_data[4] = physics_link_node_a->object;
                        exec[0] = collision_cb_b;
                        exec[1] = physics_node_base_b->attr_accessor;
                        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
            }
            physics_link_node_b = physics_link_node_b->next;
        }
        physics_link_node_a = physics_link_node_a->next;
    }

    engine_physics_apply_impulses();

    // After everything physics related is done, reset the bit array
    // used for tracking which pairs of nodes had already collided
    engine_bit_collection_erase(&collided_physics_nodes);
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