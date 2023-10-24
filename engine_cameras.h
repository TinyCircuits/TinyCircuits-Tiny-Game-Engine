#ifndef ENGINE_CAMERAS_H
#define ENGINE_CAMERAS_H

#include "utility/linked_list.h"
#include "nodes/camera_node.h"
#include "py/obj.h"

// A linked list of cameras to loop through to render
// all object layers for each camera
linked_list engine_cameras;

// The main undeletable engine camera
mp_obj_t engine_main_camera;

void engine_camera_init();

#endif  // ENGINE_CAMERAS_H