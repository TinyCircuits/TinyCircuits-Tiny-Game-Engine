#ifndef ENGINE_PHYSICS_SHAPE_TEST_H
#define ENGINE_PHYSICS_SHAPE_TEST_H

#include "engine_physics_shape_rectangle.h"
#include "engine_physics_shape_circle.h"
#include "engine_physics_shape_convex.h"
#include "engine_physics_manifold.h"
#include "math/vector2.h"
#include <math.h>

typedef struct physics_interval_t {
    mp_float_t min;
    mp_float_t max;
    mp_float_t v_min_x;
    mp_float_t v_min_y;
    mp_float_t v_max_x;
    mp_float_t v_max_y;
} physics_interval_t;

void physics_rectangle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m);
void physics_circle_circle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b, physics_manifold_class_obj_t* m);
void physics_circle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m);
physics_interval_t physics_rectangle_project(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, const vector2_class_obj_t* axis);
physics_interval_t physics_circle_project(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* axis);
physics_interval_t physics_convex_project(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, const vector2_class_obj_t* axis);
void physics_convex_convex_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_convex_class_obj_t* b, physics_manifold_class_obj_t* m);
void physics_convex_circle_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b, physics_manifold_class_obj_t* m);
void physics_convex_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m);


#endif // ENGINE_PHYSICS_SHAPE_TEST_H
