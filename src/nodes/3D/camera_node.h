#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include "py/obj.h"
#include "utility/linked_list.h"


// Node the defines view that the world is rendered about
typedef struct{
    mp_obj_t position;              // Vector3: xyz position of this node
    mp_obj_t rotation;              // Vector3: rotation of this node in space
    mp_obj_t viewport;              // Rectangle: position, width and height in screen buffer
    mp_obj_t zoom;                  // float: factor to scale drawing nodes by and also scale translation
    mp_obj_t fov;                   // Only applies to certain nodes, like voxelspace (units are radians in that case)
    mp_obj_t view_distance;         // Only applies to certain nodes, like voxelspace (units are pixels in that case)
}engine_camera_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    linked_list_node *camera_list_node;
}engine_camera_node_common_data_t;

extern const mp_obj_type_t engine_camera_node_class_type;

#endif  // CAMERA_NODE_H