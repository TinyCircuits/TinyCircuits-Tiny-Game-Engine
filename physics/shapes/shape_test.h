#include "engine_physics_shape_rectangle.h"
#include "engine_physics_shape_circle.h"
#include "engine_physics_shape_convex.h"
#include "engine_physics_manifold.h"
#include <math.h>

void physics_rectangle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    //physics_manifold_class_obj_t ret = const_separated_manifold;

    mp_float_t dx1 = a_pos->x + a->width - b_pos->x;
    mp_float_t dx2 = b_pos->x + b->width - a_pos->x;
    mp_float_t dy1 = a_pos->y + a->height - b_pos->y;
    mp_float_t dy2 = b_pos->y + b->height - a_pos->y;
    if((dx1 < 0) || (dx2 < 0) || (dy1 < 0) || (dy2 < 0)) {
        m->nrm_x = 0;
        m->nrm_y = 0;
        m->mtv_x = 0;
        m->mtv_y = 0;
        m->con_x = 0;
        m->con_y = 0;
        return;
    }
    const mp_float_t min = fminf(dx1, fminf(dx2, fminf(dy1, dy2)));
    if(dx1 == min) {
        m->mtv_x = dx1;
        m->mtv_y = 0;
        m->con_x = (a_pos->x + a->width + b_pos->x) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 1;
        m->nrm_y = 0;
    } else if(dx2 == min) {
        m->mtv_x = -dx2;
        m->mtv_y = 0;
        m->con_x = (b_pos->x + b->width + a_pos->x) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = -1;
        m->nrm_y = 0;
    } else if(dy1 == min) {
        m->mtv_x = 0;
        m->mtv_y = dy1;
        m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (a_pos->y + a->height + b_pos->y) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 0;
        m->nrm_y = 1;
    } else {
        m->mtv_x = 0;
        m->mtv_y = -dy2;
        m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (b_pos->y + b->height + a_pos->y) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 0;
        m->nrm_y = -1;
    }
}

void physics_circle_circle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    // physics_manifold_class_obj_t ret = const_separated_manifold;
    mp_float_t dx = a_pos->x - b_pos->x;
    mp_float_t dy = a_pos->y - b_pos->y;
    mp_float_t rsum = a->radius + b->radius;
    if((dx*dx + dy*dy) > (rsum*rsum)) {
        m->nrm_x = 0;
        m->nrm_y = 0;
        m->mtv_x = 0;
        m->mtv_y = 0;
        m->con_x = 0;
        m->con_y = 0;
        return;
    }
    // m->nrm_x = 0;
    // m->nrm_y = 0;
    // m->mtv_x = 0;
    // m->mtv_y = 0;
    // m->con_x = 0;
    // m->con_y = 0;
    mp_float_t dist = sqrt(dx*dx+dy*dy);
    //ENGINE_INFO_PRINTF("distance is %f\n\r", dist);
    mp_float_t idist = (dist == 0.0) ? 0.0 : 1.0/dist;
    m->nrm_x = dx*idist;
    m->nrm_y = dy*idist;
    m->mtv_x = m->nrm_x * (rsum-dist);
    m->mtv_y = m->nrm_y * (rsum-dist);
    m->con_x = a_pos->x - m->nrm_x * (a->radius + dist - b->radius) * 0.5;
    m->con_y = a_pos->y - m->nrm_y * (a->radius + dist - b->radius) * 0.5;
}

void physics_circle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    // physics_manifold_class_obj_t ret = const_separated_manifold;

    mp_float_t dx = a_pos->x - fmaxf(b_pos->x, fminf(a_pos->x, b_pos->x + b->width));
    mp_float_t dy = a_pos->y - fmaxf(b_pos->y, fminf(a_pos->y, b_pos->y + b->height));
    if((dx*dx + dy*dy) > (a->radius*a->radius)) {
        m->nrm_x = 0;
        m->nrm_y = 0;
        m->mtv_x = 0;
        m->mtv_y = 0;
        m->con_x = 0;
        m->con_y = 0;
        return;
    }
    // m->nrm_x = 0;
    // m->nrm_y = 0;
    // m->mtv_x = 0;
    // m->mtv_y = 0;
    // m->con_x = 0;
    // m->con_y = 0;
    mp_float_t dist = sqrt(dx*dx+dy*dy);
    //ENGINE_INFO_PRINTF("distance is %f\n\r", dist);
    mp_float_t idist = (dist == 0.0) ? 0.0 : 1.0/dist;
    m->nrm_x = dx*idist;
    m->nrm_y = dy*idist;
    m->mtv_x = m->nrm_x*(a->radius-dist);
    m->mtv_y = m->nrm_y*(a->radius-dist);
    m->con_x = a_pos->x - m->nrm_x*(a->radius + dist)*0.5;
    m->con_y = a_pos->y - m->nrm_y*(a->radius + dist)*0.5;
}

typedef struct physics_interval_t {
    mp_float_t min;
    mp_float_t max;
} physics_interval_t;

physics_interval_t physics_rectangle_project(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY};
    mp_float_t dot = axis->x * a_pos->x + axis->y * a_pos->y;
    ret.min = fminf(ret.min, dot);
    ret.max = fmaxf(ret.max, dot);
    dot = axis->x * (a_pos->x + a->width) + axis->y * a_pos->y;
    ret.min = fminf(ret.min, dot);
    ret.max = fmaxf(ret.max, dot);
    dot = axis->x * (a_pos->x + a->width) + axis->y * (a_pos->y + a->height);
    ret.min = fminf(ret.min, dot);
    ret.max = fmaxf(ret.max, dot);
    dot = axis->x * a_pos->x + axis->y * (a_pos->y + a->height);
    ret.min = fminf(ret.min, dot);
    ret.max = fmaxf(ret.max, dot);
    return ret;

}

physics_interval_t physics_circle_project(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY};
    mp_float_t dot = axis->x * a_pos->x + axis->y * a_pos->y;
    ret.min = dot - a->radius;
    ret.max = dot + a->radius;
    return ret;
}

physics_interval_t physics_convex_project(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY};
    mp_obj_t* vs;
    size_t vs_len;
    mp_obj_list_get(a->v_list, &vs_len, &vs);
    for(int i = 0; i < vs_len; i++) {
        vector2_class_obj_t* v = MP_OBJ_TO_PTR(vs[i]);
        mp_float_t dot = v->x * axis->x + v->y * axis->y;
        ret.min = fminf(ret.min, dot);
        ret.max = fmaxf(ret.max, dot);
    }
    return ret;
}
