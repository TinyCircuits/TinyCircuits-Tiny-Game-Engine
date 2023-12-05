#include "engine_physics_shape_rectangle.h"
#include "engine_physics_shape_circle.h"
#include "engine_physics_manifold.h"

physics_manifold_class_obj_t physics_rectangle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b) {
    physics_manifold_class_obj_t ret = const_separated_manifold;

    mp_float_t dx1 = a_pos->x + a->width - b_pos->x;
    mp_float_t dx2 = b_pos->x + b->width - a_pos->x;
    mp_float_t dy1 = a_pos->y + a->height - b_pos->y;
    mp_float_t dy2 = b_pos->y + b->height - a_pos->y;
    if((dx1 < 0) || (dx2 < 0) || (dy1 < 0) || (dy2 < 0)) return ret;
    const mp_float_t min = fminf(dx1, fminf(dx2, fminf(dy1, dy2)));
    if(dx1 == min) {
        ret.mtv_x = dx1;
        ret.mtv_y = 0;
        ret.con_x = (a_pos->x + a->width + b_pos->x) * 0.5;
        ret.con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * 0.5f;
        ret.nrm_x = 1;
        ret.nrm_y = 0;
    } else if(dx2 == min) {
        ret.mtv_x = -dx2;
        ret.mtv_y = 0;
        ret.con_x = (b_pos->x + b->width + a_pos->x) * 0.5;
        ret.con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * 0.5f;
        ret.nrm_x = -1;
        ret.nrm_y = 0;
    } else if(dy1 == min) {
        ret.mtv_x = 0;
        ret.mtv_y = dy1;
        ret.con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * 0.5f;
        ret.con_y = (a_pos->y + a->height + b_pos->y) * 0.5;
        ret.nrm_x = 0;
        ret.nrm_y = 1;
    } else {
        ret.mtv_x = 0;
        ret.mtv_y = -dy2;
        ret.con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * 0.5f;
        ret.con_y = (b_pos->y + b->height + a_pos->y) * 0.5;
        ret.nrm_x = 0;
        ret.nrm_y = -1;
    }
    return ret;
}

physics_manifold_class_obj_t physics_circle_circle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b) {
    physics_manifold_class_obj_t ret = const_separated_manifold;
    mp_float_t dx = a_pos->x - b_pos->x;
    mp_float_t dy = a_pos->y - b_pos->y;
    mp_float_t rsum = a->radius + b->radius;
    if((dx*dx + dy*dy) > (rsum*rsum)) return ret;
    mp_float_t dist = sqrtf(dx*dx+dy*dy);
    mp_float_t idist = 1.0/dist;
    ret.nrm_x = dx*idist;
    ret.nrm_y = dy*idist;
    ret.mtv_x = ret.nrm_x * (rsum-dist);
    ret.mtv_y = ret.nrm_y * (rsum-dist);
    ret.con_x = a_pos->x - ret.nrm_x * (a->radius + dist - b->radius) * 0.5;
    ret.con_y = a_pos->y - ret.nrm_y * (a->radius + dist - b->radius) * 0.5;
    return ret;
}

physics_manifold_class_obj_t physics_circle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b) {
    physics_manifold_class_obj_t ret = const_separated_manifold;

    mp_float_t dx = a_pos->x - fmaxf(b_pos->x, fminf(a_pos->x, b_pos->x + b->width));
    mp_float_t dy = a_pos->y - fmaxf(b_pos->y, fminf(a_pos->y, b_pos->y + b->height));
    if((dx*dx + dy*dy) > (a->radius*a->radius)) return ret;
    mp_float_t dist = sqrtf(dx*dx+dy*dy);
    mp_float_t idist = 1.0/dist;
    ret.nrm_x = dx*idist;
    ret.nrm_y = dy*idist;
    ret.mtv_x = ret.nrm_x*(a->radius-dist);
    ret.mtv_y = ret.nrm_y*(a->radius-dist);
    ret.con_x = a_pos->x - ret.nrm_x*(a->radius + dist)*0.5;
    ret.con_y = a_pos->y - ret.nrm_y*(a->radius + dist)*0.5;
    return ret;
}
