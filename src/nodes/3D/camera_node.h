#ifndef CAMERA_NODE_H
#define CAMERA_NODE_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "math/vector3.h"
#include "utility/engine_mp.h"
#include "nodes/node_base.h"

#define CGLM_CLIPSPACE_INCLUDE_ALL 1
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE 1
#include "../lib/cglm/include/cglm/cglm.h"
#include "../lib/cglm/include/cglm/vec3.h"
#include "../lib/cglm/include/cglm/mat4.h"
#include "../lib/cglm/include/cglm/cam.h"
#include "../lib/cglm/include/cglm/euler.h"


// Node the defines view that the world is rendered about
typedef struct{
    mp_obj_t position;              // Vector3: xyz position of this node
    mp_obj_t rotation;              // Vector3: rotation of this node in space
    mp_obj_t zoom;                  // float: factor to scale drawing nodes by and also scale translation
    mp_obj_t viewport;              // Rectangle: position, width and height in screen buffer
    mp_obj_t fov;                   // Only applies to certain nodes, like voxelspace (units are radians in that case)
    mp_obj_t view_distance;         // Only applies to certain nodes, like voxelspace (units are pixels in that case)
    mp_obj_t opacity;               // Opacity to apply to all nodes rendered by this camera
    mp_obj_t tick_cb;
    linked_list_node *camera_list_node;

    mat4 m_translation;
    mat4 m_rotation;

    mat4 m_projection;
    vec4 v_viewport;
}engine_camera_node_class_obj_t;

extern const mp_obj_type_t engine_camera_node_class_type;

// For each camera instance that is not disabled,
// pass each camera to the draw callback
void engine_camera_draw_for_each_obj(mp_obj_t dest[2]);

void engine_camera_draw_for_each(void (*draw_cb)(mp_obj_t, mp_obj_t), engine_node_base_t *node_base);

// Scale passed position and rotation due to camera zoom and rotation
void engine_camera_transform_2d(mp_obj_t camera_node, float *px, float *py, float *rotation);

#endif  // CAMERA_NODE_H