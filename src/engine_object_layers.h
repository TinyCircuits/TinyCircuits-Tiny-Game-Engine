#ifndef ENGINE_OBJECT_LAYERS_H
#define ENGINE_OBJECT_LAYERS_H

#include "utility/linked_list.h"

void engine_objects_clear_all();
void engine_objects_clear_deletable();
uint16_t engine_get_total_object_count();
linked_list_node *engine_add_object_to_layer(void *obj, uint8_t layer_index);
void engine_remove_object_from_layer(linked_list_node *object_list_node, uint8_t layer_index);
void engine_invoke_all_node_callbacks(float dt);

#endif  // ENGINE_OBJECT_LAYERS_H