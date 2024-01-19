#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2D/physics_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "nodes/2D/physics_2d_node.h"
#include "collision_shapes/circle_collision_shape_2d.h"
#include "collision_shapes/rectangle_collision_shape_2d.h"
#include "collision_shapes/polygon_collision_shape_2d.h"
#include "collision_contact_2d.h"

#include <float.h>

linked_list engine_physics_nodes;
float engine_physics_gravity_x = 0.0f;
float engine_physics_gravity_y = -0.00981f;

float engine_physics_fps_limit_period_ms = 16.667f;
float engine_physics_fps_time_at_last_tick_ms = 0.0f;


// https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=A%20helper%20method%20to%20do%20this%20might%20be%3A
void engine_physics_find_min_max_projection(float position_x, float position_y, vector2_class_obj_t *vertices, uint16_t vertex_count, float axis_x, float axis_y, float *min, float *max){
    float projection = engine_math_dot_product(position_x+vertices[0].x, position_y+vertices[0].y, axis_x, axis_y);
    *min = projection;
    *max = projection;

    for(uint16_t ivx=1; ivx<vertex_count; ivx++){
        projection = engine_math_dot_product(position_x+vertices[ivx].x, position_y+vertices[ivx].y, axis_x, axis_y);
        
        if(*min < projection){
            *min = *min;
        }else{
            *min = projection;
        }

        if(*max > projection){
            *max = *max;
        }else{
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

        if(mp_obj_is_type(collision_shape_obj_a, &circle_collision_shape_2d_class_type) &&
           mp_obj_is_type(collision_shape_obj_b, &circle_collision_shape_2d_class_type)){    // Circle vs. Circle: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=must%20be%20extended.-,Circle%20vs%20Circle,-Lets%20start%20with

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
            float full_radius = collision_shape_a->radius + collision_shape_b->radius;
            float distance_between_squared = full_radius * full_radius;

            // If true, not colliding and should stop here
            if(normal_length_squared > distance_between_squared){
                return false;
            }

            // Circles are colliding, compute information for CollisionContact2D
            float distance_between = sqrt(distance_between_squared);
            *collision_normal_x = 1.0f;
            *collision_normal_y = 0.0f;
            *collision_normal_penetration = collision_shape_a->radius;

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

                *collision_normal_penetration = full_radius - distance_between;
            }

            return true;
        }else if(mp_obj_is_type(collision_shape_obj_a, &rectangle_collision_shape_2d_class_type) &&
                 mp_obj_is_type(collision_shape_obj_b, &rectangle_collision_shape_2d_class_type)){      // Rectangle vs. Rectangle: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=Here%20is%20a%20full%20algorithm%20for%20AABB%20to%20AABB%20manifold%20generation%20and%20collision%20detection%3A

            rectangle_collision_shape_2d_class_obj_t *collision_shape_a = collision_shape_obj_a;
            rectangle_collision_shape_2d_class_obj_t *collision_shape_b = collision_shape_obj_b;

            float collision_shape_a_half_width = collision_shape_a->width / 2;
            float collision_shape_a_half_height = collision_shape_a->height / 2;

            float collision_shape_b_half_width = collision_shape_b->width / 2;
            float collision_shape_b_half_height = collision_shape_b->height / 2;

            float collision_shape_a_pos_x = collision_shape_a->position->x + physics_node_a_position->x;
            float collision_shape_a_pos_y = collision_shape_a->position->y + physics_node_a_position->y;

            float collision_shape_b_pos_x = collision_shape_b->position->x + physics_node_b_position->x;
            float collision_shape_b_pos_y = collision_shape_b->position->y + physics_node_b_position->y;

            // Normal vector from A to B
            float normal_x = collision_shape_b_pos_x - collision_shape_a_pos_x;
            float normal_y = collision_shape_b_pos_y - collision_shape_a_pos_y;

            // Calculate overlap on x axis 
            float x_overlap_length = collision_shape_a_half_width + collision_shape_b_half_width - fabsf(normal_x);

            // // SAT test on x axis
            if(x_overlap_length > 0){
                // Calculate overlap on y axis 
                float y_overlap_length = collision_shape_a_half_height + collision_shape_b_half_height - fabsf(normal_y);

                // SAT test on y axis
                if(y_overlap_length > 0){
                    if(x_overlap_length > y_overlap_length){
                        if(normal_y < 0){
                            *collision_normal_x = 0.0f;
                            *collision_normal_y = -1.0f;
                            *collision_contact_y = collision_shape_a_pos_y - collision_shape_a_half_height;
                        }else{
                            *collision_normal_x = 0.0f;
                            *collision_normal_y = 1.0f;
                            *collision_contact_y = collision_shape_a_pos_y + collision_shape_a_half_height;
                        }
                        *collision_normal_penetration = y_overlap_length;

                        // At least in the positive x, clip x at a min and
                        // max that reflects the actual endpoint positions
                        // of the overlapping area
                        float overlap_x_min = fmaxf(collision_shape_a_pos_x-collision_shape_a_half_width, collision_shape_b_pos_x-collision_shape_b_half_width);
                        float overlap_x_max = fminf(collision_shape_a_pos_x+collision_shape_a_half_width, collision_shape_b_pos_x+collision_shape_b_half_width);
                        *collision_contact_x = overlap_x_min + fabsf(overlap_x_min - overlap_x_max)/2;
                        return true;
                    }else{
                        if(normal_x < 0){
                            *collision_normal_x = -1.0f;
                            *collision_normal_y = 0.0f;
                            *collision_contact_x = collision_shape_a_pos_x - collision_shape_a_half_width;
                        }else{
                            *collision_normal_x = 1.0f;
                            *collision_normal_y = 0.0f;
                            *collision_contact_x = collision_shape_a_pos_x + collision_shape_a_half_width;
                        }
                        *collision_normal_penetration = x_overlap_length;

                        // At least in the positive y, clip x at a min and
                        // max that reflects the actual endpoint positions
                        // of the overlapping area
                        float overlap_y_min = fmaxf(collision_shape_a_pos_y-collision_shape_a_half_height, collision_shape_b_pos_y-collision_shape_b_half_height);
                        float overlap_y_max = fminf(collision_shape_a_pos_y+collision_shape_a_half_height, collision_shape_b_pos_y+collision_shape_b_half_height);
                        *collision_contact_y = overlap_y_min + fabsf(overlap_y_min - overlap_y_max)/2;
                        return true;
                    }
                }
            }
        }else if((mp_obj_is_type(collision_shape_obj_a, &rectangle_collision_shape_2d_class_type) &&
                  mp_obj_is_type(collision_shape_obj_b, &circle_collision_shape_2d_class_type))   ||
                  mp_obj_is_type(collision_shape_obj_a, &circle_collision_shape_2d_class_type) &&
                  mp_obj_is_type(collision_shape_obj_b, &rectangle_collision_shape_2d_class_type)){     // Circle vs. Rectangle or Rectangle vs. Circle

            rectangle_collision_shape_2d_class_obj_t *collision_rectangle = NULL;
            circle_collision_shape_2d_class_obj_t *collision_circle = NULL;

            if(mp_obj_is_type(collision_shape_obj_a, &rectangle_collision_shape_2d_class_type)){
                collision_rectangle = collision_shape_obj_a;
                collision_circle = collision_shape_obj_b;
            }else{
                collision_rectangle = collision_shape_obj_b;
                collision_circle = collision_shape_obj_a;
            }

            float collision_rectangle_half_width = collision_rectangle->width / 2;
            float collision_rectangle_half_height = collision_rectangle->height / 2;

            float collision_rectangle_pos_x = collision_rectangle->position->x + physics_node_a_position->x;
            float collision_rectangle_pos_y = collision_rectangle->position->y + physics_node_a_position->y;

            float collision_circle_pos_x = collision_circle->position->x + physics_node_b_position->x;
            float collision_circle_pos_y = collision_circle->position->y + physics_node_b_position->y;

            // Normal vector from A to B
            float normal_x = collision_circle_pos_x - collision_rectangle_pos_x;
            float normal_y = collision_circle_pos_y - collision_rectangle_pos_y;

            // Closest point on A to center of B
            float closest_x = engine_math_clamp(normal_x, -collision_rectangle_half_width, collision_rectangle_half_width);
            float closest_y = engine_math_clamp(normal_y, -collision_rectangle_half_height, collision_rectangle_half_height);

            bool inside = false;
            
            // Floating-point equalities? Not sure what's going on here exactly: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=25-,if(n%20%3D%3D%20closest),-26
            // Circle is inside the AABB, so we need to clamp the circle's center 
            // to the closest edge
            if(engine_math_compare_floats(normal_x, closest_x) && engine_math_compare_floats(normal_y, closest_y)){
                inside = true;

                if(fabsf(normal_x) > fabsf(normal_y)){
                    if(closest_x > 0){
                        closest_x = collision_rectangle_half_width;
                    }else{
                        closest_x = -collision_rectangle_half_width;
                    }
                }else{
                    if(closest_x > 0){
                        closest_y = collision_rectangle_half_height;
                    }else{
                        closest_y = -collision_rectangle_half_height;
                    }
                }
            }

            // Not sure about this: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=Vec2%20normal%20%3D%20n%20%2D%20closest
            float closest_normal_x = normal_x - closest_x;
            float closest_normal_y = normal_y - closest_y;
            float normal_length_squared = (closest_normal_x*closest_normal_x) + (closest_normal_y*closest_normal_y);

            // Early out of the radius is shorter than distance to closest point and 
            // Circle not inside the AABB
            if(normal_length_squared > collision_circle->radius*collision_circle->radius && !inside){
                return false;
            }

            float normal_length = sqrt(normal_length_squared);

            if(inside){
                *collision_normal_x = -closest_normal_x / normal_length;
                *collision_normal_y = -closest_normal_y / normal_length;
            }else{
                *collision_normal_x = closest_normal_x / normal_length;
                *collision_normal_y = closest_normal_y / normal_length;
            }
            *collision_normal_penetration = collision_circle->radius - normal_length;

            // Figure out collision point, seems to be working
            if(mp_obj_is_type(collision_shape_obj_a, &rectangle_collision_shape_2d_class_type)){
                *collision_contact_x = -(*collision_normal_x) * collision_circle->radius + collision_circle_pos_x;
                *collision_contact_y = -(*collision_normal_y) * collision_circle->radius + collision_circle_pos_y;
            }else{
                *collision_contact_x = *collision_normal_x * collision_circle->radius + collision_rectangle_pos_x;
                *collision_contact_y = *collision_normal_y * collision_circle->radius + collision_rectangle_pos_y;
            }

            return true;
        }else if(mp_obj_is_type(collision_shape_obj_a, &polygon_collision_shape_2d_class_type) &&
                 mp_obj_is_type(collision_shape_obj_b, &polygon_collision_shape_2d_class_type)){    // Polygon vs. Polygon: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
            polygon_collision_shape_2d_class_obj_t *polygon_shape_a = collision_shape_obj_a;
            polygon_collision_shape_2d_class_obj_t *polygon_shape_b = collision_shape_obj_b;

            float collision_shape_a_pos_x = polygon_shape_a->position->x + physics_node_a_position->x;
            float collision_shape_a_pos_y = polygon_shape_a->position->y + physics_node_a_position->y;

            float collision_shape_b_pos_x = polygon_shape_b->position->x + physics_node_b_position->x;
            float collision_shape_b_pos_y = polygon_shape_b->position->y + physics_node_b_position->y;

            float axis_x = 0.0f;
            float axis_y = 0.0f;
            float min0 = 0.0f;
            float max0 = 0.0f;
            float min1 = 0.0f;
            float max1 = 0.0f;
            *collision_normal_penetration = FLT_MAX;

            // https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=it%20would%20look%20like%20something%20like%20this%3A
            for(uint16_t inx=0; inx<polygon_shape_a->normals->len; inx++){
                axis_x = ((vector2_class_obj_t*)polygon_shape_a->normals->items[inx])->x;
                axis_y = ((vector2_class_obj_t*)polygon_shape_a->normals->items[inx])->y;
                engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, polygon_shape_a->vertices->items, polygon_shape_a->vertices->len, axis_x, axis_y, &min0, &max0);
                engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, polygon_shape_b->vertices->items, polygon_shape_b->vertices->len, axis_x, axis_y, &min1, &max1);

                if(max0 < min1 || max1 < min0){
                    // No collision
                    return false;
                }else{
                    // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
                    float overlap = fabsf(fminf(max0, max1) - fmaxf(min0, min1));
                    if(overlap < *collision_normal_penetration){
                        *collision_normal_penetration = overlap;
                        *collision_normal_x = axis_x;
                        *collision_normal_y = axis_y;
                    }
                }
            }
            for(uint16_t inx=0; inx<polygon_shape_b->normals->len; inx++){
                axis_x = ((vector2_class_obj_t*)polygon_shape_b->normals->items[inx])->x;
                axis_y = ((vector2_class_obj_t*)polygon_shape_b->normals->items[inx])->y;
                engine_physics_find_min_max_projection(collision_shape_a_pos_x, collision_shape_a_pos_y, polygon_shape_a->vertices->items, polygon_shape_a->vertices->len, axis_x, axis_y, &min0, &max0);
                engine_physics_find_min_max_projection(collision_shape_b_pos_x, collision_shape_b_pos_y, polygon_shape_b->vertices->items, polygon_shape_b->vertices->len, axis_x, axis_y, &min1, &max1);

                if(max0 < min1 || max1 < min0){
                    // No collision
                    return false;
                }else{
                    // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
                    float overlap = fabsf(fminf(max0, max1) - fmaxf(min0, min1));
                    if(overlap < *collision_normal_penetration){
                        *collision_normal_penetration = overlap;
                        *collision_normal_x = axis_x;
                        *collision_normal_y = axis_y;
                    }
                }
            }

            // ENGINE_FORCE_PRINTF("%.03f %.03f %.03f", *collision_contact_x, *collision_contact_y, *collision_normal_penetration);
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
                float collision_normal_penetration = 0.0f;

                if(engine_physics_check_collision(physics_node_base_a, physics_node_base_b, &collision_normal_x, &collision_normal_y, &collision_contact_x, &collision_contact_y, &collision_normal_penetration)){
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
                    // NOTE: DO NOT NEED TO DO mp_store_attr since we're modifying the pointers!
                    if(physics_node_a_dynamic){
                        physics_node_a_position->x -= collision_normal_x * collision_normal_penetration / 2;
                        physics_node_a_position->y -= collision_normal_y * collision_normal_penetration / 2;
                    }

                    if(physics_node_b_dynamic){
                        physics_node_b_position->x += collision_normal_x * collision_normal_penetration / 2;
                        physics_node_b_position->y += collision_normal_y * collision_normal_penetration / 2;
                    }

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