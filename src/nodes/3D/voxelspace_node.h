#ifndef VOXELSPACE_NODE_H
#define VOXELSPACE_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// https://github.com/s-macke/VoxelSpace
typedef struct{
    mp_obj_t position;              // Vector3
    mp_obj_t texture_resource;      // TextureResource: how the ground looks
    mp_obj_t heightmap_resource;    // TextureResource: how tall the ground looks
    mp_obj_t rotation;              // Vector3
    mp_obj_t scale;
    mp_obj_t repeat;
    mp_obj_t flip;
    mp_obj_t lod;
    mp_obj_t tick_cb;
}engine_voxelspace_node_class_obj_t;

extern const mp_obj_type_t engine_voxelspace_node_class_type;
void voxelspace_node_class_draw(engine_node_base_t *voxelspace_node_base, mp_obj_t camera_node);

#endif  // VOXELSPACE_NODE_H