#ifndef ENGINE_GUI_H
#define ENGINE_GUI_H

#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "py/obj.h"

linked_list_node *engine_gui_track(engine_node_base_t *obj);
void engine_gui_untrack(linked_list_node *gui_list_node);
void engine_gui_clear_all();

bool engine_gui_get_focus();
void engine_gui_toggle_focus();
engine_node_base_t *engine_gui_get_focused();

void engine_gui_tick();

#endif  // ENGINE_GUI_H