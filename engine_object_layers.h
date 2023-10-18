#ifndef ENGINE_OBJECT_LAYERS_H
#define ENGINE_OBJECT_LAYERS_H

#include "utility/linked_list.h"

linked_list_node *engine_add_object_to_layer(void *obj, uint16_t layer_index);
void engine_remove_object_from_layer(linked_list_node *object_list_node, uint16_t layer_index);
void engine_invoke_all_node_callbacks();


#endif  // ENGINE_OBJECT_LAYERS_H