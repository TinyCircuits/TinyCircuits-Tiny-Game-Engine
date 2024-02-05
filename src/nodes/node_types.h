#ifndef NODE_TYPES_H
#define NODE_TYPES_H

#include <stdint.h>

#define NODE_TYPE_EMPTY             0
#define NODE_TYPE_CAMERA            1

#define NODE_TYPE_PHYSICS_2D        2   // Node that falls, has velocity and/or acceleration

#define NODE_TYPE_RECTANGLE_2D      3
#define NODE_TYPE_LINE_2D           4
#define NODE_TYPE_CIRCLE_2D         5
#define NODE_TYPE_SPRITE_2D         6
#define NODE_TYPE_POLYGON_2D        7

#define NODE_TYPE_VOXELSPACE        8   // https://github.com/s-macke/VoxelSpace


#endif  // NODE_TYPES_H