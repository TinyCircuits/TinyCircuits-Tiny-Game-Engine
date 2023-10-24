#ifndef ENGINE_CAMERAS_H
#define ENGINE_CAMERAS_H

#include "utility/linked_list.h"
#include "nodes/camera_node.h"
#include "py/obj.h"

// A linked list of cameras to loop through to render
// all object layers for each camera
static linked_list engine_cameras;

// Cameras are nodes just like everything else but are tracked
// new a different list that the engine can loop through quickly.
// This avoids a search through all the nodes
linked_list_node *engine_camera_track(engine_camera_node_class_obj_t *obj);

// Call this with the saved link_list_node pointer to
// remove the camera from the list of tracked cameras
void engine_camera_untrack(linked_list_node *camera_list_node);


#endif  // ENGINE_CAMERAS_H
