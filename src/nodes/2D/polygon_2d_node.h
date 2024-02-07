#ifndef POLYGON_2D_NODE_H
#define POLYGON_2D_NODE_H

#include "py/obj.h"

// A polygon 2d node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t vertices;  // List of vertices
    mp_obj_t color;     // The color of this polygon
    mp_obj_t outline;   // bool: draw outlined if True (default), otherwise filled (TODO: implement filled, hard...)
    mp_obj_t rotation;  // polygon rotation in radians
    mp_obj_t scale;     // float: how much to scale up the polygon (uses average postion of verts for origin)
}engine_polygon_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_polygon_2d_node_common_data_t;

extern const mp_obj_type_t engine_polygon_2d_node_class_type;


#endif  // POLYGON_2D_NODE_H
