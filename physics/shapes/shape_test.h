#ifndef ENGINE_PHYSICS_SHAPE_TEST_H
#define ENGINE_PHYSICS_SHAPE_TEST_H

#include "engine_physics_shape_rectangle.h"
#include "engine_physics_shape_circle.h"
#include "engine_physics_shape_convex.h"
#include "engine_physics_manifold.h"
#include <math.h>

STATIC void physics_rectangle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    //physics_manifold_class_obj_t ret = const_separated_manifold;
    vector2_class_obj_t a_old = *a_pos;
    vector2_class_obj_t b_old = *b_pos;
    a_pos->x -= a->width * 0.5;
    a_pos->y -= a->height * 0.5;
    b_pos->x -= b->width * 0.5;
    b_pos->y -= b->height * 0.5;
    mp_float_t dx1 = a_pos->x + a->width - b_pos->x;
    mp_float_t dx2 = b_pos->x + b->width - a_pos->x;
    mp_float_t dy1 = a_pos->y + a->height - b_pos->y;
    mp_float_t dy2 = b_pos->y + b->height - a_pos->y;
    if((dx1 < 0) || (dx2 < 0) || (dy1 < 0) || (dy2 < 0)) {
        *m = const_separated_manifold;
        *a_pos = a_old;
        *b_pos = b_old;
        return;
    }
    const mp_float_t min = fminf(dx1, fminf(dx2, fminf(dy1, dy2)));
    if(dx1 == min) {
        m->mtv_x = -dx1;
        m->mtv_y = 0;
        m->con_x = (a_pos->x + a->width + b_pos->x) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = -1;
        m->nrm_y = 0;
    } else if(dx2 == min) {
        m->mtv_x = dx2;
        m->mtv_y = 0;
        m->con_x = (b_pos->x + b->width + a_pos->x) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->height, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 1;
        m->nrm_y = 0;
    } else if(dy1 == min) {
        m->mtv_x = 0;
        m->mtv_y = -dy1;
        m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (a_pos->y + a->height + b_pos->y) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 0;
        m->nrm_y = -1;
    } else {
        m->mtv_x = 0;
        m->mtv_y = dy2;
        m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->width, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
        m->con_y = (b_pos->y + b->height + a_pos->y) * MICROPY_FLOAT_CONST(0.5);
        m->nrm_x = 0;
        m->nrm_y = 1;
    }
    *a_pos = a_old;
    *b_pos = b_old;
}

STATIC void physics_circle_circle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    // physics_manifold_class_obj_t ret = const_separated_manifold;
    mp_float_t dx = a_pos->x - b_pos->x;
    mp_float_t dy = a_pos->y - b_pos->y;
    mp_float_t rsum = a->radius + b->radius;
    if((dx*dx + dy*dy) > (rsum*rsum)) {
        *m = const_separated_manifold;
        return;
    }
    mp_float_t dist2 = dx*dx+dy*dy;
    //ENGINE_INFO_PRINTF("distance is %f\n\r", dist);
    if(dist2 != 0.0) {
        mp_float_t dist = MICROPY_FLOAT_C_FUN(sqrt)(dist2);
        mp_float_t idist = 1.0/dist;
        m->nrm_x = dx*idist;
        m->nrm_y = dy*idist;
        m->mtv_x = m->nrm_x * (rsum-dist);
        m->mtv_y = m->nrm_y * (rsum-dist);
        m->con_x = a_pos->x - m->nrm_x * (a->radius + dist - b->radius) * 0.5;
        m->con_y = a_pos->y - m->nrm_y * (a->radius + dist - b->radius) * 0.5;
    } else {
        *m = const_separated_manifold;
    }
}

STATIC void physics_circle_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m) {
    // physics_manifold_class_obj_t ret = const_separated_manifold;

    mp_float_t dx = a_pos->x - fmaxf(b_pos->x, fminf(a_pos->x, b_pos->x + b->width));
    mp_float_t dy = a_pos->y - fmaxf(b_pos->y, fminf(a_pos->y, b_pos->y + b->height));
    mp_float_t dist2 = dx*dx+dy*dy;
    if(dist2 > (a->radius*a->radius)) {
        // Distance to nearest point on AABB is outside of the radius
        *m = const_separated_manifold;

    } else if(dist2 != 0.0) {
        // Work out normal and contact
        mp_float_t dist = MICROPY_FLOAT_C_FUN(sqrt)(dist2);
        mp_float_t idist = 1.0/dist;
        m->nrm_x = dx*idist;
        m->nrm_y = dy*idist;
        m->mtv_x = m->nrm_x*(a->radius-dist);
        m->mtv_y = m->nrm_y*(a->radius-dist);
        m->con_x = a_pos->x - m->nrm_x*(a->radius + dist)*0.5;
        m->con_y = a_pos->y - m->nrm_y*(a->radius + dist)*0.5;

    } else {
        // Circle center is inside rectangle
        // physics_shape_rectangle_class_obj_t a_rect;
        // a_rect.width = a_rect.height = 0;
        // vector2_class_obj_t a_rect_pos;
        // a_rect_pos.x = a_pos->x;
        // a_rect_pos.y = a_pos->y;
        // physics_rectangle_rectangle_test(&a_rect_pos, &a_rect, b_pos, b, m);
        mp_float_t dx1 = a_pos->x - b_pos->x;
        mp_float_t dx2 = b_pos->x + b->width - a_pos->x;
        mp_float_t dy1 = a_pos->y - b_pos->y;
        mp_float_t dy2 = b_pos->y + b->height - a_pos->y;
        if((dx1 < 0) || (dx2 < 0) || (dy1 < 0) || (dy2 < 0)) {
            *m = const_separated_manifold;
            return;
        }
        const mp_float_t min = fminf(dx1, fminf(dx2, fminf(dy1, dy2)));
        if(dx1 == min) {
            m->mtv_x = -dx1 - a->radius;
            m->mtv_y = 0;
            m->con_x = (a_pos->x + a->radius + b_pos->x) * MICROPY_FLOAT_CONST(0.5);
            m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->radius, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
            m->nrm_x = -1;
            m->nrm_y = 0;
        } else if(dx2 == min) {
            m->mtv_x = dx2 + a->radius;
            m->mtv_y = 0;
            m->con_x = (b_pos->x + b->width + a_pos->x - a->radius) *  MICROPY_FLOAT_CONST(0.5);
            m->con_y = (fmaxf(a_pos->y, b_pos->y) + fminf(a_pos->y+a->radius, b_pos->y+b->height)) * MICROPY_FLOAT_CONST(0.5);
            m->nrm_x = 1;
            m->nrm_y = 0;
        } else if(dy1 == min) {
            m->mtv_x = 0;
            m->mtv_y = -dy1 - a->radius;
            m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->radius, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
            m->con_y = (a_pos->y + a->radius + b_pos->y) * MICROPY_FLOAT_CONST(0.5);
            m->nrm_x = 0;
            m->nrm_y = -1;
        } else {
            m->mtv_x = 0;
            m->mtv_y = dy2 + a->radius;
            m->con_x = (fmaxf(a_pos->x, b_pos->x) + fminf(a_pos->x+a->radius, b_pos->x+b->width)) * MICROPY_FLOAT_CONST(0.5);
            m->con_y = (b_pos->y + b->height + a_pos->y - a->radius) * MICROPY_FLOAT_CONST(0.5);
            m->nrm_x = 0;
            m->nrm_y = 1;
        }
    }
}

typedef struct physics_interval_t {
    mp_float_t min;
    mp_float_t max;
    mp_float_t v_min_x;
    mp_float_t v_min_y;
    mp_float_t v_max_x;
    mp_float_t v_max_y;
} physics_interval_t;

STATIC physics_interval_t physics_rectangle_project(vector2_class_obj_t* a_pos, physics_shape_rectangle_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY, 0, 0};
    mp_float_t dot = axis->x * a_pos->x + axis->y * a_pos->y;
    // ret.min = fminf(ret.min, dot);
    // ret.max = fmaxf(ret.max, dot);
    if(dot < ret.min) {
        ret.min = dot;
        ret.v_min_x = a_pos->x;
        ret.v_min_y = a_pos->y;
    }
    if(dot > ret.max) {
        ret.max = dot;
        ret.v_max_x = a_pos->x;
        ret.v_max_y = a_pos->y;
    }
    dot = axis->x * (a_pos->x + a->width) + axis->y * a_pos->y;
    // ret.min = fminf(ret.min, dot);
    // ret.max = fmaxf(ret.max, dot);
    if(dot < ret.min) {
        ret.min = dot;
        ret.v_min_x = a_pos->x + a->width;
        ret.v_min_y = a_pos->y;
    }
    if(dot > ret.max) {
        ret.max = dot;
        ret.v_max_x = a_pos->x + a->width;
        ret.v_max_y = a_pos->y;
    }
    dot = axis->x * (a_pos->x + a->width) + axis->y * (a_pos->y + a->height);
    // ret.min = fminf(ret.min, dot);
    // ret.max = fmaxf(ret.max, dot);
    if(dot < ret.min) {
        ret.min = dot;
        ret.v_min_x = a_pos->x + a->width;
        ret.v_min_y = a_pos->y + a->height;
    }
    if(dot > ret.max) {
        ret.max = dot;
        ret.v_max_x = a_pos->x + a->width;
        ret.v_max_y = a_pos->y + a->height;
    }
    dot = axis->x * a_pos->x + axis->y * (a_pos->y + a->height);
    // ret.min = fminf(ret.min, dot);
    // ret.max = fmaxf(ret.max, dot);
    if(dot < ret.min) {
        ret.min = dot;
        ret.v_min_x = a_pos->x;
        ret.v_min_y = a_pos->y + a->height;
    }
    if(dot > ret.max) {
        ret.max = dot;
        ret.v_max_x = a_pos->x;
        ret.v_max_y = a_pos->y + a->height;
    }
    return ret;

}

STATIC physics_interval_t physics_circle_project(vector2_class_obj_t* a_pos, physics_shape_circle_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY, 0, 0};
    mp_float_t dot = axis->x * a_pos->x + axis->y * a_pos->y;
    ret.min = dot - a->radius;
    ret.max = dot + a->radius;
    ret.v_min_x = a_pos->x - axis->x * a->radius;
    ret.v_min_y = a_pos->y - axis->y * a->radius;
    ret.v_max_x = a_pos->x + axis->x * a->radius;
    ret.v_max_y = a_pos->y + axis->y * a->radius;
    return ret;
}

STATIC physics_interval_t physics_convex_project(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* axis) {
    physics_interval_t ret = {INFINITY, -INFINITY, 0, 0};
    mp_obj_t* vs;
    size_t vs_len;
    mp_obj_list_get(a->v_t_list, &vs_len, &vs);
    for(int i = 0; i < vs_len; i++) {
        vector2_class_obj_t* v = MP_OBJ_TO_PTR(vs[i]);
        mp_float_t dot = (v->x + a_pos->x) * axis->x + (v->y + a_pos->y) * axis->y;
        //ret.min = fminf(ret.min, dot);
        if(dot < ret.min) {
            ret.min = dot;
            ret.v_min_x = v->x + a_pos->x;
            ret.v_min_y = v->y + a_pos->y;
        }
        if(dot > ret.max) {
            ret.max = dot;
            ret.v_max_x = v->x + a_pos->x;
            ret.v_max_y = v->y + a_pos->y;
        }
        //ret.max = fmaxf(ret.max, dot);
    }
    return ret;
}

STATIC void physics_convex_convex_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_convex_class_obj_t* b, physics_manifold_class_obj_t* m) {
    mp_obj_t* b_ns;
    size_t b_ns_len;
    mp_obj_list_get(b->n_t_list, &b_ns_len, &b_ns);

    mp_obj_t* b_vs;
    size_t b_vs_len;
    mp_obj_list_get(b->v_t_list, &b_vs_len, &b_vs);

    mp_obj_t* a_ns;
    size_t a_ns_len;
    mp_obj_list_get(a->n_t_list, &a_ns_len, &a_ns);

    mp_obj_t* a_vs;
    size_t a_vs_len;
    mp_obj_list_get(a->v_t_list, &a_vs_len, &a_vs);
    mp_float_t min_overlap = INFINITY;

    // Project all of A's vertices onto B's surface normals
    for(int i = 0; i < b_ns_len; i++) {
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(b_ns[i]);
        physics_interval_t a_proj = physics_convex_project(a_pos, a, n);
        physics_interval_t b_proj = physics_convex_project(b_pos, b, n);
        if(a_proj.max < b_proj.min || a_proj.min > b_proj.max) {
            // Separating axis!
            *m = const_separated_manifold;
            return;
        } else {
            if(a_proj.max - b_proj.min < min_overlap) {
                // A's most extant point along the axis is in B
                min_overlap = a_proj.max - b_proj.min;
                m->nrm_x = -n->x;
                m->nrm_y = -n->y;
                m->con_x = a_proj.v_max_x;
                m->con_y = a_proj.v_max_y;
            }
            if(b_proj.max - a_proj.min < min_overlap) {
                // A's least extant point along the axis is in B
                min_overlap = b_proj.max - a_proj.min;
                m->nrm_x = -n->x;
                m->nrm_y = -n->y;
                m->con_x = a_proj.v_min_x;
                m->con_y = a_proj.v_min_y;
            }
        }
    }

    // Project all of B's vertices onto A's surface normals
    for(int i = 0; i < a_ns_len; i++) {
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(a_ns[i]);
        physics_interval_t a_proj = physics_convex_project(a_pos, a, n);
        physics_interval_t b_proj = physics_convex_project(b_pos, b, n);
        if(b_proj.max < a_proj.min || b_proj.min > a_proj.max) {
            // Separating axis!
            *m = const_separated_manifold;
            return;
        } else {
            if(b_proj.max - a_proj.min < min_overlap) {
                // A's most extant point along the axis is in B
                min_overlap = b_proj.max - a_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_max_x;
                m->con_y = b_proj.v_max_y;
            }
            if(a_proj.max - b_proj.min < min_overlap) {
                // A's least extant point along the axis is in B
                min_overlap = a_proj.max - b_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_min_x;
                m->con_y = b_proj.v_min_y;
            }
        }
    }
    m->mtv_x = min_overlap * m->nrm_x;
    m->mtv_y = min_overlap * m->nrm_y;
}

STATIC void physics_convex_circle_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_circle_class_obj_t* b, physics_manifold_class_obj_t* m) {

    mp_obj_t* a_ns;
    size_t a_ns_len;
    mp_obj_list_get(a->n_t_list, &a_ns_len, &a_ns);

    mp_obj_t* a_vs;
    size_t a_vs_len;
    mp_obj_list_get(a->v_t_list, &a_vs_len, &a_vs);
    mp_float_t min_overlap = INFINITY;

    // Circles have infinitely many surface normals, so just use the directions from the center to the closest vertex on A
    if(a_vs_len < 1) {*m = const_separated_manifold; return; }
    vector2_class_obj_t* closest_v = MP_OBJ_TO_PTR(a_vs[0]);
    mp_float_t dx = closest_v->x - b_pos->x;
    mp_float_t dy = closest_v->y - b_pos->y;
    mp_float_t min_dist2 = dx*dx+dy*dy;
    if(min_dist2 == 0.0) min_dist2 = INFINITY;
    for(int i = 1; i < a_vs_len; i++) {
        ENGINE_INFO_PRINTF("Current closest point is (%f, %f) with mindist2 of %f\n\r", closest_v->x, closest_v->y, min_dist2);
        vector2_class_obj_t* a_v = MP_OBJ_TO_PTR(a_vs[i]);
        ENGINE_INFO_PRINTF("v is (%f, %f)\n\r", a_v->x, a_v->y);
        dx = a_v->x - b_pos->x;
        dy = a_v->y - b_pos->y;
        ENGINE_INFO_PRINTF("dx is (%f, %f)\n\r", dx, dy);
        mp_float_t dist = dx*dx + dy*dy;
        if(dist < min_dist2 && dist != 0.0) {
            min_dist2 = dist;
            closest_v = a_v;
        }
    }
    ENGINE_INFO_PRINTF("Closest point is (%f, %f) with mindist2 of %f\n\r", closest_v->x, closest_v->y, min_dist2);

    vector2_class_obj_t n;
    n.x = closest_v->x - b_pos->x;
    n.y = closest_v->y - b_pos->y;
    mp_float_t il = 1.0 / MICROPY_FLOAT_C_FUN(sqrt)(n.x*n.x + n.y*n.y);
    n.x *= il;
    n.y *= il;
    physics_interval_t a_proj = physics_convex_project(a_pos, a, &n);
    physics_interval_t b_proj = physics_circle_project(b_pos, b, &n);
    if(a_proj.max < b_proj.min || a_proj.min > b_proj.max) {
        // Separating axis!
        *m = const_separated_manifold;
        return;
    } else {
        if(a_proj.max - b_proj.min < min_overlap) {
            // A's most extant point along the axis is in B
            min_overlap = a_proj.max - b_proj.min;
            m->nrm_x = -n.x;
            m->nrm_y = -n.y;
            m->con_x = a_proj.v_max_x;
            m->con_y = a_proj.v_max_y;
        }
        if(b_proj.max - a_proj.min < min_overlap) {
            // A's least extant point along the axis is in B
            min_overlap = b_proj.max - a_proj.min;
            m->nrm_x = -n.x;
            m->nrm_y = -n.y;
            m->con_x = a_proj.v_min_x;
            m->con_y = a_proj.v_min_y;
        }
    }

    // Project B onto A's surface normals
    for(int i = 0; i < a_ns_len; i++) {
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(a_ns[i]);
        physics_interval_t a_proj = physics_convex_project(a_pos, a, n);
        physics_interval_t b_proj = physics_circle_project(b_pos, b, n);
        if(b_proj.max < a_proj.min || b_proj.min > a_proj.max) {
            // Separating axis!
            *m = const_separated_manifold;
            return;
        } else {
            if(b_proj.max - a_proj.min < min_overlap) {
                // A's most extant point along the axis is in B
                min_overlap = b_proj.max - a_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_max_x;
                m->con_y = b_proj.v_max_y;
            }
            if(a_proj.max - b_proj.min < min_overlap) {
                // A's least extant point along the axis is in B
                min_overlap = a_proj.max - b_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_min_x;
                m->con_y = b_proj.v_min_y;
            }
        }
    }
    if(min_overlap != INFINITY) {
        m->mtv_x = min_overlap * m->nrm_x;
        m->mtv_y = min_overlap * m->nrm_y;
    } else *m = const_separated_manifold;
}

STATIC void physics_convex_rectangle_test(vector2_class_obj_t* a_pos, physics_shape_convex_class_obj_t* a, vector2_class_obj_t* b_pos, physics_shape_rectangle_class_obj_t* b, physics_manifold_class_obj_t* m) {

    mp_obj_t* a_ns;
    size_t a_ns_len;
    mp_obj_list_get(a->n_t_list, &a_ns_len, &a_ns);

    mp_obj_t* a_vs;
    size_t a_vs_len;
    mp_obj_list_get(a->v_t_list, &a_vs_len, &a_vs);
    mp_float_t min_overlap = INFINITY;

    // Project all of A's vertices onto B's surface normals
    const static vector2_class_obj_t rectangle_normals[4] = {
        {{&vector2_class_type}, (mp_float_t)(-1.0), (mp_float_t)0.0},
        {{&vector2_class_type}, (mp_float_t)(0.0), (mp_float_t)-1.0},
        {{&vector2_class_type}, (mp_float_t)(1.0), (mp_float_t)0.0},
        {{&vector2_class_type}, (mp_float_t)(0.0), (mp_float_t)1.0},
    };
    for(int i = 0; i < 4; i++) {
        vector2_class_obj_t* n = &rectangle_normals[i];
        physics_interval_t a_proj = physics_convex_project(a_pos, a, n);
        physics_interval_t b_proj = physics_rectangle_project(b_pos, b, n);
        if(a_proj.max < b_proj.min || a_proj.min > b_proj.max) {
            // Separating axis!
            *m = const_separated_manifold;
            return;
        } else {
            if(a_proj.max - b_proj.min < min_overlap) {
                // A's most extant point along the axis is in B
                min_overlap = a_proj.max - b_proj.min;
                m->nrm_x = -n->x;
                m->nrm_y = -n->y;
                m->con_x = a_proj.v_max_x;
                m->con_y = a_proj.v_max_y;
            }
            if(b_proj.max - a_proj.min < min_overlap) {
                // A's least extant point along the axis is in B
                min_overlap = b_proj.max - a_proj.min;
                m->nrm_x = -n->x;
                m->nrm_y = -n->y;
                m->con_x = a_proj.v_min_x;
                m->con_y = a_proj.v_min_y;
            }
        }
    }

    // Project all of B's vertices onto A's surface normals
    for(int i = 0; i < a_ns_len; i++) {
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(a_ns[i]);
        physics_interval_t a_proj = physics_convex_project(a_pos, a, n);
        physics_interval_t b_proj = physics_rectangle_project(b_pos, b, n);
        if(b_proj.max < a_proj.min || b_proj.min > a_proj.max) {
            // Separating axis!
            *m = const_separated_manifold;
            return;
        } else {
            if(b_proj.max - a_proj.min < min_overlap) {
                // A's most extant point along the axis is in B
                min_overlap = b_proj.max - a_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_max_x;
                m->con_y = b_proj.v_max_y;
            }
            if(a_proj.max - b_proj.min < min_overlap) {
                // A's least extant point along the axis is in B
                min_overlap = a_proj.max - b_proj.min;
                m->nrm_x = n->x;
                m->nrm_y = n->y;
                m->con_x = b_proj.v_min_x;
                m->con_y = b_proj.v_min_y;
            }
        }
    }
    m->mtv_x = min_overlap * m->nrm_x;
    m->mtv_y = min_overlap * m->nrm_y;
}

#endif // ENGINE_PHYSICS_SHAPE_TEST_H
