#ifndef ENGINE_ANIMATION_MODULE_H
#define ENGINE_ANIMATION_MODULE_H

#include "engine_animation_tween.h"
#include "utility/linked_list.h"

enum engine_animation_loop_types {engine_animation_loop, engine_animation_one_shot};

linked_list_node* engine_animation_track_tween(tween_class_obj_t *tween);
void engine_animation_untrack_tween(linked_list_node *list_node);
void engine_animation_init();
void engine_animation_tick(float dt);


#endif