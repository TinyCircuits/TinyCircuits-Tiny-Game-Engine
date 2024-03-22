#ifndef VOXELSPACE_NODE_H
#define VOXELSPACE_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// https://github.com/s-macke/VoxelSpace
typedef struct{
    mp_obj_t position;              // Vector3
    mp_obj_t texture_resource;      // TextureResource: how the ground looks
    mp_obj_t heightmap_resource;    // TextureResource: how tall the ground looks
    mp_obj_t height_scale;          // Multiplier to make node taller or shorter
    mp_obj_t rotation;              // Vector3
}engine_voxelspace_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t transform_texture_pixel_cb;    // Callback that the user can define to transform pixels retrieved from 'texture_resource'
    mp_obj_t transform_heightmap_pixel_cb;  // Callback that the user can define to transform pixels retrieved from 'heightmap_resource'
}engine_voxelspace_node_common_data_t;

extern const mp_obj_type_t engine_voxelspace_node_class_type;
void voxelspace_node_class_draw(engine_node_base_t *voxelspace_node_base, mp_obj_t camera_node);

#endif  // VOXELSPACE_NODE_H