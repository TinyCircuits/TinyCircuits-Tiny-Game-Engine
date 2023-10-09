#ifndef NODE_TYPES_H
#define NODE_TYPES_H

#include <stdint.h>

#define NODE_TYPE_BASE              0
#define NODE_TYPE_PHYSICS           1

#define NODE_TYPE_BITMAP_SPRITE     2
#define NODE_TYPE_QUAD_SPRITE       3
#define NODE_TYPE_TILE_MAP          4   // Give it a list of textures and IDs, populate 2D array with IDs to draw textures

#define NODE_TYPE_VOXEL_SPACE       5   // https://github.com/s-macke/VoxelSpace
#define NODE_TYPE_MODEL             6   //
#define NODE_TYPE_RAYCAST_WALLS     7
#define NODE_TYPE_RAYCAST_FLOOR     8
#define NODE_TYPE_RAYCAST_CEILING   9


typedef struct base_node {
    uint8_t node_type = NODE_TYPE_BASE;
    uint16_t node_index = 0;  
};


#endif  // NODE_TYPES_H