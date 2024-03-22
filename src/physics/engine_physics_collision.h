#ifndef ENGINE_PHYSICS_COLLISION_H
#define ENGINE_PHYSICS_COLLISION_H

#include "nodes/2D/physics_rectangle_2d_node.h"
#include "nodes/2D/physics_circle_2d_node.h"
#include "math/vector2.h"

// Structure used to hold common data about collisions.
// Filled during collision check of various pairs of
// colliders
typedef struct{
    float collision_normal_x;
    float collision_normal_y;
    float collision_contact_x;
    float collision_contact_y;
    float collision_normal_penetration;
    float contact_velocity_magnitude;

    float moment_arm_a_x;
    float moment_arm_a_y;
    float moment_arm_b_x;
    float moment_arm_b_y;

    float relative_velocity_x;
    float relative_velocity_y;
}contact_t;


void engine_physics_get_relative_velocity(engine_physics_node_base_t *physics_node_base_a, engine_physics_node_base_t *physics_node_base_b, contact_t *contact);


// https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/index.html#:~:text=A%20helper%20method%20to%20do%20this%20might%20be%3A
// Projects vertices onto vector defined `axis_x` and `axis_y` and gets the extents of the projections
// into `min` and `max`. Used for rectangles/polygons in SAT algorithm
void engine_physics_rect_find_min_max_projection(float position_x, float position_y,
                                                 float *vertices_x, float *vertices_y,
                                                 float axis_x, float axis_y,
                                                 float *min, float *max);

// https://www.sevenson.com.au/programming/sat/#:~:text=then%20add%20and%20subtract%20the%20radius.
// Projects circle onto vector defined by `axis_x` and `axis_y` and gets the extents of the projections
// into `min` and `max`. Used for circles in SAT algorithm
void engine_physics_circle_find_min_max_projection(float position_x, float position_y,
                                                   float radius,
                                                   float axis_x, float axis_y,
                                                   float *min, float *max);

// https://dyn4j.org/2011/11/contact-points-using-clipping/#:~:text=or%20contact%20patch.-,Finding%20the%20Features,-The%20first%20step
// Used for finding which edges of polygons are in contact. Returns by modifying edge vertex positions of the
// contacting edges. Used in rectangle vs. rectanlge and rectangle vs. circle collisions
void engine_physics_rect_rect_get_contacting(float px, float py,
                                             float collision_normal_x, float collision_normal_y,
                                             float *max_proj_vertex_x, float *max_proj_vertex_y,
                                             float *edge_v0_x, float *edge_v0_y,
                                             float *edge_v1_x, float *edge_v1_y,
                                             float *vertices_x, float *vertices_y);

// Get the contact point from the OBB to OBB collision: https://dyn4j.org/2011/11/contact-points-using-clipping/
// Finds the contact point between rectangle vs. rectangle. ONce the contacting edges are found, 
// a ling segment to line segment intersection is found. If the line segments are parallel,
// the midpoint of the overlapping line segments is taken as the collision contact point
void engine_physics_rect_rect_get_contact(contact_t *contact,
                                          engine_physics_node_base_t *physics_rectangle_a,
                                          engine_physics_node_base_t *physics_rectangle_b);

// Get the contact point from the OBB to circle collision: https://dyn4j.org/2011/11/contact-points-using-clipping/
// Finds the closest vertex in the rectangle to the circle, creates an edge between the circle and that
// vertex, runs SAT, finds a direction to place contact point from circle's perspective
void engine_physics_rect_circle_get_contact(contact_t *contact,
                                            float circle_to_vert_axis_x, float circle_to_vert_axis_y,
                                            engine_physics_node_base_t *physics_node_base_rectangle,
                                            engine_physics_node_base_t *physics_node_base_circle);

// rectangle vs. rectangle: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032t
// Runs SAT to figure out if rectangles are colliding
bool engine_physics_check_rect_rect_collision(engine_physics_node_base_t *physics_node_base_a,
                                              engine_physics_node_base_t *physics_node_base_b,
                                              contact_t *contact);

// rectangle vs. circle: https://www.sevenson.com.au/programming/sat/#:~:text=to%20separate%20them.-,What%20about%20circles,-%3F
// At the end of the day, uses SAT to figure out if circle and rectangle are colliding. Does need
// to treat the circle special so that the SAT routine can be performed
bool engine_physics_check_rect_circle_collision(engine_physics_node_base_t *physics_rect_node_base,
                                                engine_physics_node_base_t *physics_circle_node_base,
                                                contact_t *contact);

// https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t#:~:text=must%20be%20extended.-,Circle%20vs%20Circle,-Lets%20start%20with
// https://github.com/RandyGaul/ImpulseEngine/blob/8d5f4d9113876f91a53cfb967879406e975263d1/Collision.cpp#L32-L66
// Simple way of testing if two circles are colliding
bool engine_physics_check_circle_circle_collision(engine_physics_node_base_t *physics_node_base_a,
                                                  engine_physics_node_base_t *physics_node_base_b,
                                                  contact_t *contact);


#endif  // ENGINE_PHYSICS_COLLISION_H