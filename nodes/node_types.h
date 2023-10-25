#ifndef NODE_TYPES_H
#define NODE_TYPES_H

#include <stdint.h>

#define NODE_TYPE_EMPTY             0
#define NODE_TYPE_CAMERA            1

#define NODE_TYPE_PHYSICS           2   // Node that falls, has velocity and/or acceleration
#define NODE_TYPE_PHYSICS_COLLIDER  3   // Node that collides (can have different shapes)

#define NODE_TYPE_BITMAP_SPRITE     4
#define NODE_TYPE_QUAD_SPRITE       5
#define NODE_TYPE_TILE_MAP          6   // Give it a list of textures and IDs, populate 2D array with IDs to draw textures

#define NODE_TYPE_VOXEL_SPACE       7   // https://github.com/s-macke/VoxelSpace
#define NODE_TYPE_MODEL             8   //
#define NODE_TYPE_RAYCAST_WALLS     9
#define NODE_TYPE_RAYCAST_FLOOR     10
#define NODE_TYPE_RAYCAST_CEILING   11


#endif  // NODE_TYPES_H