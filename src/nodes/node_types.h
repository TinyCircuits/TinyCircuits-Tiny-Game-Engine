#ifndef NODE_TYPES_H
#define NODE_TYPES_H

#include <stdint.h>

#define NODE_TYPE_EMPTY                 0
#define NODE_TYPE_CAMERA                1

#define NODE_TYPE_PHYSICS_RECTANGLE_2D  2
#define NODE_TYPE_PHYSICS_CIRCLE_2D     3

#define NODE_TYPE_RECTANGLE_2D          4
#define NODE_TYPE_LINE_2D               5
#define NODE_TYPE_CIRCLE_2D             6
#define NODE_TYPE_SPRITE_2D             7
#define NODE_TYPE_TEXT_2D               8

#define NODE_TYPE_VOXELSPACE            9   // https://github.com/s-macke/VoxelSpace
#define NODE_TYPE_MESH_3D               10  // https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/mathematics-computing-2d-coordinates-of-3d-points.html
#define NODE_TYPE_GUI_BUTTON_2D         11
#define NODE_TYPE_GUI_BITMAP_BUTTON_2D  12

#endif  // NODE_TYPES_H