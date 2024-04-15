#ifndef ENGINE_ANIMATION_MODULE_H
#define ENGINE_ANIMATION_MODULE_H

#include "engine_animation_tween.h"
#include "engine_animation_delay.h"
#include "utility/linked_list.h"

enum engine_animation_loop_types {engine_animation_loop_loop, engine_animation_loop_one_shot, engine_animation_loop_ping_pong};
enum engine_animation_ease_types {engine_animation_ease_linear,
                                  engine_animation_ease_sine_in,   engine_animation_ease_sine_out,   engine_animation_ease_sine_in_out,
                                  engine_animation_ease_quad_in,   engine_animation_ease_quad_out,   engine_animation_ease_quad_in_out,
                                  engine_animation_ease_cubic_in,  engine_animation_ease_cubic_out,  engine_animation_ease_cubic_in_out,
                                  engine_animation_ease_quart_in,  engine_animation_ease_quart_out,  engine_animation_ease_quart_in_out,
                                  engine_animation_ease_quint_in,  engine_animation_ease_quint_out,  engine_animation_ease_quint_in_out,
                                  engine_animation_ease_exp_in,    engine_animation_ease_exp_out,    engine_animation_ease_exp_in_out,
                                  engine_animation_ease_circ_in,   engine_animation_ease_circ_out,   engine_animation_ease_circ_in_out,
                                  engine_animation_ease_back_in,   engine_animation_ease_back_out,   engine_animation_ease_back_in_out,
                                  engine_animation_ease_elast_in,  engine_animation_ease_elast_out,  engine_animation_ease_elast_in_out,
                                  engine_animation_ease_bounce_in, engine_animation_ease_bounce_out, engine_animation_ease_bounce_in_out};

linked_list_node* engine_animation_track(mp_obj_t *animation_element);
void engine_animation_untrack(linked_list_node *list_node);
void engine_animation_init();
void engine_animation_tick(float dt);


#endif