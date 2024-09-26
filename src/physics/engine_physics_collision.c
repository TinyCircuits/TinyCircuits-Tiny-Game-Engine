#include "physics/engine_physics_collision.h"
#include "math/engine_math.h"
#include <stdint.h>
#include <float.h>
#include "draw/engine_display_draw.h"


void engine_physics_setup_contact(physics_contact_t *contact){
    contact->collision_normal_x = 0.0f;
    contact->collision_normal_y = 0.0f;
    contact->collision_contact_x = 0.0f;
    contact->collision_contact_y = 0.0f;
    contact->collision_normal_penetration = 0.0f;
    contact->moment_arm_a_x = 0.0f;
    contact->moment_arm_a_y = 0.0f;
    contact->moment_arm_b_x = 0.0f;
    contact->moment_arm_b_y = 0.0f;
    contact->contact_velocity_magnitude = 0.0f;
    contact->relative_velocity_x = 0.0f;
    contact->relative_velocity_y = 0.0f;
}


void engine_physics_setup_abs_rectangle(engine_node_base_t *node_base, physics_abs_rectangle_t *abs_rect){
    abs_rect->node_base = node_base;
    engine_physics_node_base_t *physics_rect = node_base->node;

    engine_inheritable_2d_t inherited;
    node_base_inherit_2d(node_base, &inherited);
    abs_rect->abs_x = inherited.px;
    abs_rect->abs_y = inherited.py;
    abs_rect->rotation = inherited.rotation;

    engine_physics_rectangle_2d_node_calculate(physics_rect, abs_rect->vertices_x, abs_rect->vertices_y, abs_rect->normals_x, abs_rect->normals_y, abs_rect->rotation);
    abs_rect->dynamic = mp_obj_get_int(physics_rect->dynamic);
}


void engine_physics_setup_abs_circle(engine_node_base_t *node_base, physics_abs_circle_t *abs_circle){
    abs_circle->node_base = node_base;
    engine_physics_node_base_t *physics_circle = node_base->node;
    engine_physics_circle_2d_node_class_obj_t *circle = physics_circle->unique_data;

    engine_inheritable_2d_t inherited;
    node_base_inherit_2d(node_base, &inherited);
    abs_circle->abs_x = inherited.px;
    abs_circle->abs_y = inherited.py;
    abs_circle->rotation = inherited.rotation;

    float scale_radius_by = 1.0f;
    if(inherited.sx < inherited.sy){
        scale_radius_by = inherited.sx;
    }else{
        scale_radius_by = inherited.sy;
    }

    abs_circle->radius = mp_obj_get_float(circle->radius) * scale_radius_by;
    abs_circle->dynamic = mp_obj_get_int(physics_circle->dynamic);
}


// Some algorithms, like SAT, pick the first normal they come across
// as the collision normal. Need to figure out the real direction
// https://stackoverflow.com/a/6244218
void engine_physics_resolve_normal_direction(float abs_a_position_x, float abs_a_position_y, float abs_b_position_x, float abs_b_position_y, physics_contact_t *contact){
    float a_to_b_direction_x = abs_b_position_x - abs_a_position_x;
    float a_to_b_direction_y = abs_b_position_y - abs_a_position_y;

    if(engine_math_dot_product(contact->collision_normal_x, contact->collision_normal_y, a_to_b_direction_x, a_to_b_direction_y) >= 0.0f){
        contact->collision_normal_x = -contact->collision_normal_x;
        contact->collision_normal_y = -contact->collision_normal_y;
    }
}


// If either node is not dynamic, make sure its velocity is set to zero
void engine_physics_cancel_dynamics(engine_physics_node_base_t *physics_node_base_a, engine_physics_node_base_t *physics_node_base_b){
    vector2_class_obj_t *physics_node_a_velocity = physics_node_base_a->velocity;
    vector2_class_obj_t *physics_node_b_velocity = physics_node_base_b->velocity;

    bool physics_node_a_dynamic = mp_obj_get_int(physics_node_base_a->dynamic);
    bool physics_node_b_dynamic = mp_obj_get_int(physics_node_base_b->dynamic);

    // If either node is not dynamic, set any velocities to zero no matter what set to
    if(!physics_node_a_dynamic){
        physics_node_a_velocity->x.value = 0.0f;
        physics_node_a_velocity->y.value = 0.0f;
    }

    if(!physics_node_b_dynamic){
        physics_node_b_velocity->x.value = 0.0f;
        physics_node_b_velocity->y.value = 0.0f;
    }
}


// https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
void engine_physics_get_relative_velocity(engine_physics_node_base_t *physics_node_base_a, engine_physics_node_base_t *physics_node_base_b, physics_contact_t *contact){
    vector2_class_obj_t *physics_node_a_velocity = physics_node_base_a->velocity;
    vector2_class_obj_t *physics_node_b_velocity = physics_node_base_b->velocity;

    float relative_velocity_x = physics_node_b_velocity->x.value - physics_node_a_velocity->x.value;
    float relative_velocity_y = physics_node_b_velocity->y.value - physics_node_a_velocity->y.value;

    contact->contact_velocity_magnitude = engine_math_dot_product(relative_velocity_x, relative_velocity_y, contact->collision_normal_x, contact->collision_normal_y);



    // // https://github.com/RandyGaul/ImpulseEngine/blob/master/Manifold.cpp#L65-L92
    // // https://github.com/victorfisac/Physac/blob/29d9fc06860b54571a02402fff6fa8572d19bd12/src/physac.h#L1671-L1679
    // vector2_class_obj_t *a_position = physics_node_base_a->position;
    // vector2_class_obj_t *b_position = physics_node_base_b->position;

    // vector2_class_obj_t *a_velocity = physics_node_base_a->velocity;
    // vector2_class_obj_t *b_velocity = physics_node_base_b->velocity;

    // contact->moment_arm_a_x = contact->collision_contact_x - a_position->x;
    // contact->moment_arm_a_y = contact->collision_contact_y - a_position->y;

    // contact->moment_arm_b_x = contact->collision_contact_x - b_position->x;
    // contact->moment_arm_b_y = contact->collision_contact_y - b_position->y;

    // engine_draw_pixel(0b1111100000000000, contact->collision_contact_x+64, contact->collision_contact_y+64);
    // engine_draw_line(0b0000011111100000, 64+a_position->x, 64+a_position->y, 64+a_position->x+contact->moment_arm_a_x, 64+a_position->y+contact->moment_arm_a_y, NULL);
    // engine_draw_line(0b0000000000011111, 64+b_position->x, 64+b_position->y, 64+b_position->x+contact->moment_arm_b_x, 64+b_position->y+contact->moment_arm_b_y, NULL);

    // ENGINE_FORCE_PRINTF("%.03f %.03f %.03f %.03f", contact->moment_arm_a_x, contact->moment_arm_a_y, contact->moment_arm_b_x, contact->moment_arm_b_y);

    // // https://github.com/tutsplus/ImpulseEngine/blob/7a66ba78562efd9c3029ff0992d21b31026afc8a/Manifold.cpp#L69-L74
    // float cross_a_x = 0.0f;
    // float cross_a_y = 0.0f;
    // float cross_b_x = 0.0f;
    // float cross_b_y = 0.0f;

    // engine_math_cross_product_float_v(physics_node_base_a->angular_velocity, contact->moment_arm_a_x, contact->moment_arm_a_y, &cross_a_x, &cross_a_y);
    // engine_math_cross_product_float_v(physics_node_base_b->angular_velocity, contact->moment_arm_b_x, contact->moment_arm_b_x, &cross_b_x, &cross_b_y);
    // // engine_math_cross_product_v_float(contact->moment_arm_a_x, contact->moment_arm_a_y, physics_node_base_a->angular_velocity, &cross_a_x, &cross_a_y);
    // // engine_math_cross_product_v_float(contact->moment_arm_b_x, contact->moment_arm_b_x, physics_node_base_b->angular_velocity, &cross_b_x, &cross_b_y);

    // contact->relative_velocity_x = (b_velocity->x + cross_b_x) - (a_velocity->x + cross_a_x);
    // contact->relative_velocity_y = (b_velocity->y + cross_b_y) - (a_velocity->y + cross_a_y);

    // contact->contact_velocity_magnitude = engine_math_dot_product(contact->relative_velocity_x, contact->relative_velocity_y, contact->collision_normal_x, contact->collision_normal_y);
}


// https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
bool engine_physics_check_velocities_separating(physics_contact_t *contact){
    if(contact->contact_velocity_magnitude <= 0.0f){
        return true;
    }else{
        return false;
    }
}


void engine_physics_rect_find_min_max_projection(float position_x, float position_y, float *vertices_x, float *vertices_y, float axis_x, float axis_y, float *min, float *max){
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


void engine_physics_circle_find_min_max_projection(float position_x, float position_y, float radius, float axis_x, float axis_y, float *min, float *max){
    float projection = engine_math_dot_product(position_x, position_y, axis_x, axis_y);
    *min = projection - radius;
    *max = projection + radius;
}


void engine_physics_rect_rect_get_contacting(float px, float py, float collision_normal_x, float collision_normal_y, float *max_proj_vertex_x, float *max_proj_vertex_y, float *edge_v0_x, float *edge_v0_y, float *edge_v1_x, float *edge_v1_y, float *vertices_x, float *vertices_y){
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


void engine_physics_rect_rect_get_contact(physics_contact_t *contact, float abs_a_position_x, float abs_a_position_y, float abs_b_position_x, float abs_b_position_y, engine_physics_node_base_t *physics_node_base_a, engine_physics_node_base_t *physics_node_base_b){
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
    
    float node_a_vertices_x[4];
    float node_a_vertices_y[4];

    float node_a_normals_x[2];
    float node_a_normals_y[2];

    float node_b_vertices_x[4];
    float node_b_vertices_y[4];

    float node_b_normals_x[2];
    float node_b_normals_y[2];

    engine_physics_rectangle_2d_node_calculate(physics_node_base_a, node_a_vertices_x, node_a_vertices_y, node_a_normals_x, node_a_normals_y, 0.0f);
    engine_physics_rectangle_2d_node_calculate(physics_node_base_b, node_b_vertices_x, node_b_vertices_y, node_b_normals_x, node_b_normals_y, 0.0f);

    engine_physics_rect_rect_get_contacting(abs_a_position_x, abs_a_position_y, -contact->collision_normal_x, -contact->collision_normal_y, &a_max_proj_vertex_x, &a_max_proj_vertex_y, &a_edge_v0_x, &a_edge_v0_y, &a_edge_v1_x, &a_edge_v1_y, node_a_vertices_x, node_a_vertices_y);
    engine_physics_rect_rect_get_contacting(abs_b_position_x, abs_b_position_y,  contact->collision_normal_x,  contact->collision_normal_y, &b_max_proj_vertex_x, &b_max_proj_vertex_y, &b_edge_v0_x, &b_edge_v0_y, &b_edge_v1_x, &b_edge_v1_y, node_b_vertices_x, node_b_vertices_y);

    // Already know that the intersection will happen, just need to handle case
    // where the two edges can be exactly parallel. Project edges onto this parallel
    // line, find the smallest portion, get the midpoint, use that as the intersection
    if(engine_math_2d_do_segments_intersect(a_edge_v0_x, a_edge_v0_y, a_edge_v1_x, a_edge_v1_y, b_edge_v0_x, b_edge_v0_y, b_edge_v1_x, b_edge_v1_y, &contact->collision_contact_x, &contact->collision_contact_y) == false){
        float to_project_x[] = {a_edge_v0_x, a_edge_v1_x, b_edge_v0_x, b_edge_v1_x};
        float to_project_y[] = {a_edge_v0_y, a_edge_v1_y, b_edge_v0_y, b_edge_v1_y};

        // Make this project vector perpendicular
        float projections[4];
        projections[0] = engine_math_dot_product(a_edge_v0_x, a_edge_v0_y, contact->collision_normal_y, -contact->collision_normal_x);
        projections[1] = engine_math_dot_product(a_edge_v1_x, a_edge_v1_y, contact->collision_normal_y, -contact->collision_normal_x);
        projections[2] = engine_math_dot_product(b_edge_v0_x, b_edge_v0_y, contact->collision_normal_y, -contact->collision_normal_x);
        projections[3] = engine_math_dot_product(b_edge_v1_x, b_edge_v1_y, contact->collision_normal_y, -contact->collision_normal_x);

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
        engine_math_2d_midpoint(to_project_x[1], to_project_y[1], to_project_x[2], to_project_y[2], &contact->collision_contact_x, &contact->collision_contact_y);
    }
}


void engine_physics_rect_circle_get_contact(physics_contact_t *contact, float circle_to_vert_axis_x, float circle_to_vert_axis_y, float abs_rectangle_pos_x, float abs_rectangle_pos_y, float abs_circle_pos_x, float abs_circle_pos_y, engine_physics_node_base_t *physics_node_base_rectangle, engine_physics_node_base_t *physics_node_base_circle){
    engine_physics_circle_2d_node_class_obj_t *physics_circle = physics_node_base_circle->unique_data;

    float a_max_proj_vertex_x = 0.0f;
    float a_max_proj_vertex_y = 0.0f;

    float a_edge_v0_x = 0.0f;
    float a_edge_v0_y = 0.0f;
    float a_edge_v1_x = 0.0f;
    float a_edge_v1_y = 0.0f;

    float node_rect_vertices_x[4];
    float node_rect_vertices_y[4];

    float node_rect_normals_x[2];
    float node_rect_normals_y[2];

    engine_physics_rectangle_2d_node_calculate(physics_node_base_rectangle, node_rect_vertices_x, node_rect_vertices_y, node_rect_normals_x, node_rect_normals_y, 0.0f);

    engine_physics_rect_rect_get_contacting(abs_rectangle_pos_x, abs_rectangle_pos_y, -contact->collision_normal_x, -contact->collision_normal_y, &a_max_proj_vertex_x, &a_max_proj_vertex_y, &a_edge_v0_x, &a_edge_v0_y, &a_edge_v1_x, &a_edge_v1_y, node_rect_vertices_x, node_rect_vertices_y);
    
    float circle_radius = mp_obj_get_float(physics_circle->radius);

    float circle_pos_proj = engine_math_dot_product(abs_circle_pos_x, abs_circle_pos_y, contact->collision_normal_y, -contact->collision_normal_x);
    float rect_extend_proj_0 = engine_math_dot_product(a_edge_v0_x, a_edge_v0_y, contact->collision_normal_y, -contact->collision_normal_x);
    float rect_extend_proj_1 = engine_math_dot_product(a_edge_v1_x, a_edge_v1_y, contact->collision_normal_y, -contact->collision_normal_x);

    if(circle_pos_proj < rect_extend_proj_0 || circle_pos_proj > rect_extend_proj_1){
        contact->collision_contact_x = abs_circle_pos_x + circle_to_vert_axis_x * circle_radius;
        contact->collision_contact_y = abs_circle_pos_y + circle_to_vert_axis_y * circle_radius;
    }else{
        contact->collision_contact_x = abs_circle_pos_x + contact->collision_normal_x * circle_radius;
        contact->collision_contact_y = abs_circle_pos_y + contact->collision_normal_y * circle_radius;
    }
}


bool engine_physics_check_rect_rect_collision(physics_abs_rectangle_t *abs_rect_a, physics_abs_rectangle_t *abs_rect_b, physics_contact_t *contact){
    float axis_x = 0.0f;
    float axis_y = 0.0f;
    float a_min = 0.0f;
    float a_max = 0.0f;
    float b_min = 0.0f;
    float b_max = 0.0f;
    contact->collision_normal_penetration = FLT_MAX;

    // https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=it%20would%20look%20like%20something%20like%20this%3A
    // Only need to test two axes since rectangles have parallel directions: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/2D-rotated-rectangle-collision-r2604/#:~:text=This%20gives%20us-,four%20axes,-%2C%20each%20of%20which
    for(uint8_t inx=0; inx<2; inx++){
        axis_x = abs_rect_a->normals_x[inx];
        axis_y = abs_rect_a->normals_y[inx];
        engine_physics_rect_find_min_max_projection(abs_rect_a->abs_x, abs_rect_a->abs_y, abs_rect_a->vertices_x, abs_rect_a->vertices_y, axis_x, axis_y, &a_min, &a_max);
        engine_physics_rect_find_min_max_projection(abs_rect_b->abs_x, abs_rect_b->abs_y, abs_rect_b->vertices_x, abs_rect_b->vertices_y, axis_x, axis_y, &b_min, &b_max);

        if(a_max < b_min || b_max < a_min){
            // No collision
            return false;
        }else{
            // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
            float overlap = fminf(a_max, b_max) - fmaxf(a_min, b_min);
            if(overlap < contact->collision_normal_penetration){
                contact->collision_normal_penetration = overlap;
                contact->collision_normal_x = axis_x;
                contact->collision_normal_y = axis_y;
            }
        }
    }
    for(uint8_t inx=0; inx<2; inx++){
        axis_x = abs_rect_b->normals_x[inx];
        axis_y = abs_rect_b->normals_y[inx];
        engine_physics_rect_find_min_max_projection(abs_rect_a->abs_x, abs_rect_a->abs_y, abs_rect_a->vertices_x, abs_rect_a->vertices_y, axis_x, axis_y, &a_min, &a_max);
        engine_physics_rect_find_min_max_projection(abs_rect_b->abs_x, abs_rect_b->abs_y, abs_rect_b->vertices_x, abs_rect_b->vertices_y, axis_x, axis_y, &b_min, &b_max);

        if(a_max < b_min || b_max < a_min){
            // No collision
            return false;
        }else{
            // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
            float overlap = fminf(a_max, b_max) - fmaxf(a_min, b_min);
            if(overlap < contact->collision_normal_penetration){
                contact->collision_normal_penetration = overlap;
                contact->collision_normal_x = axis_x;
                contact->collision_normal_y = axis_y;
            }
        }
    }

    // Resolve collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
    engine_physics_resolve_normal_direction(abs_rect_a->abs_x, abs_rect_a->abs_y, abs_rect_b->abs_x, abs_rect_b->abs_y, contact);
    engine_physics_cancel_dynamics(abs_rect_a->node_base->node, abs_rect_b->node_base->node);
    engine_physics_rect_rect_get_contact(contact, abs_rect_a->abs_x, abs_rect_a->abs_y, abs_rect_b->abs_x, abs_rect_b->abs_y, abs_rect_a->node_base->node, abs_rect_b->node_base->node);
    engine_physics_get_relative_velocity(abs_rect_a->node_base->node, abs_rect_b->node_base->node, contact);

    // Do not resolve if velocities are separating (this does mean
    // objects inside each other will not collide until a non separating
    // velocity is set)
    if((abs_rect_a->dynamic || abs_rect_b->dynamic) && engine_physics_check_velocities_separating(contact)){
        return false;
    }

    return true;
}


bool engine_physics_check_rect_circle_collision(physics_abs_rectangle_t *abs_rect, physics_abs_circle_t *abs_circle, physics_contact_t *contact){
    // Need to find closest vertex on rect to circle
    float min_distance = FLT_MAX;
    float closest_delta_x = 0;
    float closest_delta_y = 0;

    for(uint8_t ivx=0; ivx<4; ivx++){
        float vert_x = abs_rect->abs_x + abs_rect->vertices_x[ivx];
        float vert_y = abs_rect->abs_y + abs_rect->vertices_y[ivx];
        
        // Get the position difference between rect vertex and circle
        // (rect vertices are rotated when the rect's rotation changes)
        float delta_x = vert_x - abs_rect->abs_x;
        float delta_y = vert_y - abs_rect->abs_y;

        // Don't need true distance from pythagorean's theorem,
        // just want smallest so don't need sqrt
        float distance = (delta_x * delta_x) + (delta_y * delta_y);

        // Check if we found a closer vertex to the circle
        if(distance < min_distance){
            min_distance = distance;
            closest_delta_x = delta_x;
            closest_delta_y = delta_y;
        }
    }

    float circle_to_vert_axis_x = 0.0f;
    float circle_to_vert_axis_y = 0.0f;

    // Convert closet delta into unit vector
    float mag = sqrtf((closest_delta_x*closest_delta_x) + (closest_delta_y*closest_delta_y));
    if(mag == 0.0f){
        ENGINE_FORCE_PRINTF("rect vs. circle: mag == 0.0...");
        circle_to_vert_axis_x = 0.0f;
        circle_to_vert_axis_y = 0.0f;
    }else{
        circle_to_vert_axis_x = closest_delta_x / mag;
        circle_to_vert_axis_y = closest_delta_y / mag;
    }

    // Now that we have an axis to test against with the circle, do SAT
    float axis_x = 0.0f;
    float axis_y = 0.0f;
    float a_min = 0.0f;
    float a_max = 0.0f;
    float b_min = 0.0f;
    float b_max = 0.0f;
    contact->collision_normal_penetration = FLT_MAX;

    // https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=it%20would%20look%20like%20something%20like%20this%3A
    // Only need to test two axes since rectangles have parallel directions: https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/2D-rotated-rectangle-collision-r2604/#:~:text=This%20gives%20us-,four%20axes,-%2C%20each%20of%20which
    for(uint8_t inx=0; inx<2; inx++){
        axis_x = abs_rect->normals_x[inx];
        axis_y = abs_rect->normals_y[inx];
        engine_physics_rect_find_min_max_projection(abs_rect->abs_x, abs_rect->abs_y, abs_rect->vertices_x, abs_rect->vertices_y, axis_x, axis_y, &a_min, &a_max);
        engine_physics_circle_find_min_max_projection(abs_circle->abs_x, abs_circle->abs_y, abs_circle->radius, axis_x, axis_y, &b_min, &b_max);

        if(a_max < b_min || b_max < a_min){
            // No collision
            return false;
        }else{
            // https://dyn4j.org/2010/01/sat/#:~:text=MTV%20when%20the-,shapes%20intersect.,-1%0A2%0A3
            float overlap = fminf(a_max, b_max) - fmaxf(a_min, b_min);
            if(overlap < contact->collision_normal_penetration){
                contact->collision_normal_penetration = overlap;
                contact->collision_normal_x = axis_x;
                contact->collision_normal_y = axis_y;
            }
        }
    }

    // Resolve collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
    engine_physics_resolve_normal_direction(abs_rect->abs_x, abs_rect->abs_y, abs_circle->abs_x, abs_circle->abs_y, contact);
    engine_physics_cancel_dynamics(abs_rect->node_base->node, abs_circle->node_base->node);
    engine_physics_rect_circle_get_contact(contact, circle_to_vert_axis_x, circle_to_vert_axis_y, abs_rect->abs_x, abs_rect->abs_y, abs_circle->abs_x, abs_circle->abs_y, abs_rect->node_base->node, abs_circle->node_base->node);
    engine_physics_get_relative_velocity(abs_rect->node_base->node, abs_circle->node_base->node, contact);

    // Do not resolve if velocities are separating (this does mean
    // objects inside each other will not collide until a non separating
    // velocity is set)
    if((abs_rect->dynamic || abs_circle->dynamic) && engine_physics_check_velocities_separating(contact)){
        return false;
    }

    return true;
}


bool engine_physics_check_circle_circle_collision(physics_abs_circle_t *abs_circle_a, physics_abs_circle_t *abs_circle_b, physics_contact_t *contact){
    // Get the normal/vector of translation
    float normal_x = abs_circle_b->abs_x - abs_circle_a->abs_x;
    float normal_y = abs_circle_b->abs_y - abs_circle_a->abs_y;

    float normal_length_squared = (normal_x*normal_x) + (normal_y*normal_y);
    float radius = abs_circle_a->radius + abs_circle_b->radius;

    // If true, not close enough to be in contact, no collision
    if(normal_length_squared >= radius * radius){
        return false;
    }

    float normal_length = sqrtf(normal_length_squared);

    if(normal_length == 0.0f){
        contact->collision_normal_penetration = abs_circle_a->radius;
        contact->collision_normal_x = 1.0f;
        contact->collision_normal_y = 0.0f;
        contact->collision_contact_x = abs_circle_a->abs_x;
        contact->collision_contact_y = abs_circle_a->abs_y;
    }else{
        contact->collision_normal_penetration = radius - normal_length;
        contact->collision_normal_x = normal_x / normal_length;
        contact->collision_normal_y = normal_y / normal_length;
        contact->collision_contact_x = contact->collision_normal_x * abs_circle_a->radius + abs_circle_a->abs_x;
        contact->collision_contact_y = contact->collision_normal_y * abs_circle_a->radius + abs_circle_a->abs_y;
    }

    // Resolve collision: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=more%20readable%20than%20mathematical%20notation!
    engine_physics_resolve_normal_direction(abs_circle_a->abs_x, abs_circle_a->abs_y, abs_circle_b->abs_x, abs_circle_b->abs_y, contact);
    engine_physics_cancel_dynamics(abs_circle_a->node_base->node, abs_circle_b->node_base->node);
    engine_physics_get_relative_velocity(abs_circle_a->node_base->node, abs_circle_b->node_base->node, contact);

    // Do not resolve if velocities are separating (this does mean
    // objects inside each other will not collide until a non separating
    // velocity is set)
    if((abs_circle_a->dynamic || abs_circle_b->dynamic) && engine_physics_check_velocities_separating(contact)){
        return false;
    }

    return true;
}