#ifndef ENGINE_CAMERAS_H
#define ENGINE_CAMERAS_H

#include "utility/linked_list.h"
#include "nodes/3d/camera_node.h"
#include "nodes/node_base.h"
#include "py/obj.h"

// Cameras are nodes just like everything else but are tracked
// new a different list that the engine can loop through quickly.
// This avoids a search through all the nodes
linked_list_node *engine_camera_track(engine_node_base_t *obj);

// Call this with the saved link_list_node pointer to
// remove the camera from the list of tracked cameras
void engine_camera_untrack(linked_list_node *camera_list_node);

// For each camera instance that is not disabled,
// pass each camera to the draw callback
void engine_camera_draw_for_each(mp_obj_t dest[2]);

// Remove all cameras from linked list
void engine_camera_clear_all();


#endif  // ENGINE_CAMERAS_H