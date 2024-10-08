#include "engine_object_layers.h"
#include "nodes/empty_node.h"
#include "nodes/3D/camera_node.h"
#include "nodes/3D/voxelspace_node.h"
#include "nodes/3D/voxelspace_sprite_node.h"
#include "nodes/3D/mesh_node.h"
#include "nodes/2D/rectangle_2d_node.h"
#include "nodes/2D/line_2d_node.h"
#include "nodes/2D/circle_2d_node.h"
#include "nodes/2D/sprite_2d_node.h"
#include "nodes/2D/text_2d_node.h"
#include "nodes/2D/gui_button_2d_node.h"
#include "nodes/2D/gui_bitmap_button_2d_node.h"
#include "nodes/2D/physics_rectangle_2d_node.h"
#include "nodes/2D/physics_circle_2d_node.h"
#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "engine_collections.h"

#include "py/gc.h"

uint16_t engine_object_layer_count = 128;
linked_list engine_object_layers[128];


void engine_objects_clear_all(){
    ENGINE_INFO_PRINTF("Untracking all nodes...");
    for(uint8_t inx=0; inx<engine_object_layer_count; inx++){

        while(engine_object_layers[inx].start != NULL){
            engine_node_base_t *node_base = engine_object_layers[inx].start->object;

            // m_del_obj does not call finalizer, call it ourselves then delete the mp object
            mp_obj_t final = mp_load_attr(node_base, MP_QSTR___del__);
            mp_call_function_0(final);
            m_del_obj(mp_obj_get_type(node_base), node_base);
        }
    }
}


void engine_objects_clear_deletable(){
    linked_list *deletable = engine_collections_get_deletable_list();

    while(deletable->start != NULL){
        engine_node_base_t *node_base = ((linked_list_node*)(deletable->start))->object;

        // m_del_obj does not call finalizer, call it ourselves then delete the mp object
        mp_obj_t final = mp_load_attr(node_base, MP_QSTR___del__);
        mp_call_function_0(final);
        m_del_obj(mp_obj_get_type(node_base), node_base);
    }
}


uint16_t engine_get_total_object_count(){
    uint16_t count = 0;
    for(uint8_t ilx=0; ilx<engine_object_layer_count; ilx++){
        count += engine_object_layers[ilx].count;
    }

    return count;
}


// Add an object to the pool of all nodes in 'engine_object_layers' at some layer
linked_list_node *engine_add_object_to_layer(void *obj, uint8_t layer_index){
    if(layer_index >= engine_object_layer_count){
        ENGINE_ERROR_PRINTF("Tried to add object to layer %d but the max layer index is %d. Resize the number of available draw layers at the cost of memory", layer_index, engine_object_layer_count-1);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tried to add object to layer index that is out of bounds! Resize the object layer count!"));
    }

    return linked_list_add_obj(&engine_object_layers[layer_index], obj);
}


void engine_remove_object_from_layer(linked_list_node *object_list_node, uint8_t layer_index){
    linked_list_del_list_node(&engine_object_layers[layer_index], object_list_node);
}


// Go through all nodes and call their tick callbacks depending on the
// node type. For example, some nodes will only have a 'tick()'
// dt_s - delta time in seconds
void engine_invoke_all_node_tick_callbacks(float dt_s){
    linked_list_node *current_linked_list_node = NULL;

    for(uint16_t ilx=0; ilx<engine_object_layer_count; ilx++){
        ENGINE_INFO_PRINTF("Starting ticking nodes in layer %d/%d", ilx, engine_object_layer_count-1);

        current_linked_list_node = engine_object_layers[ilx].start;

        while(current_linked_list_node != NULL){
            // Get the base node that every node is stored under
            engine_node_base_t *node_base = current_linked_list_node->object;

            mp_obj_t exec[3];

            switch(node_base->type){
                case NODE_TYPE_EMPTY:
                {
                    engine_empty_node_class_obj_t *empty_node = node_base->node;
                    if(empty_node->tick_cb != mp_const_none){
                        exec[0] = empty_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_CAMERA:
                {
                    engine_camera_node_class_obj_t *camera_node = node_base->node;
                    if(camera_node->tick_cb != mp_const_none){
                        exec[0] = camera_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_VOXELSPACE:
                {
                    engine_voxelspace_node_class_obj_t *voxelspace_node= node_base->node;
                    if(voxelspace_node->tick_cb != mp_const_none){
                        exec[0] = voxelspace_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_VOXELSPACE_SPRITE:
                {
                    engine_voxelspace_sprite_node_class_obj_t *voxelspace_sprite_node= node_base->node;
                    if(voxelspace_sprite_node->tick_cb != mp_const_none){
                        exec[0] = voxelspace_sprite_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_MESH_3D:
                {
                    engine_mesh_node_class_obj_t *mesh_node = node_base->node;
                    if(mesh_node->tick_cb != mp_const_none){
                        exec[0] = mesh_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_RECTANGLE_2D:
                {
                    engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = node_base->node;
                    if(rectangle_2d_node->tick_cb != mp_const_none){
                        exec[0] = rectangle_2d_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_LINE_2D:
                {
                    engine_line_2d_node_class_obj_t *line_2d_node = node_base->node;
                    if(line_2d_node->tick_cb != mp_const_none){
                        exec[0] = line_2d_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_CIRCLE_2D:
                {
                    engine_circle_2d_node_class_obj_t *circle_2d_node = node_base->node;
                    if(circle_2d_node->tick_cb != mp_const_none){
                        exec[0] = circle_2d_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_SPRITE_2D:
                {
                    engine_sprite_2d_node_class_obj_t *sprite_2d_node = node_base->node;
                    if(sprite_2d_node->tick_cb != mp_const_none){
                        exec[0] = sprite_2d_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_TEXT_2D:
                {
                    engine_text_2d_node_class_obj_t *text_2d_node = node_base->node;
                    if(text_2d_node->tick_cb != mp_const_none){
                        exec[0] = text_2d_node->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_GUI_BUTTON_2D:
                {
                    engine_gui_button_2d_node_class_obj_t *button_2d_node = node_base->node;
                    exec[1] = node_base->attr_accessor;

                    if(button_2d_node->tick_cb != mp_const_none){
                        exec[0] = button_2d_node->tick_cb;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }

                    if(button_2d_node->on_focused_cb != mp_const_none && button_2d_node->focused == true){
                        exec[0] = button_2d_node->on_focused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(button_2d_node->on_just_focused_cb != mp_const_none && button_2d_node->last_focused == false && button_2d_node->focused == true){
                        exec[0] = button_2d_node->on_just_focused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(button_2d_node->on_just_unfocused_cb != mp_const_none && button_2d_node->last_focused == true && button_2d_node->focused == false){
                        exec[0] = button_2d_node->on_just_unfocused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(button_2d_node->on_pressed_cb != mp_const_none && button_2d_node->pressed == true){
                        exec[0] = button_2d_node->on_pressed_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(button_2d_node->on_just_pressed_cb != mp_const_none && button_2d_node->last_pressed == false && button_2d_node->pressed == true){
                        exec[0] = button_2d_node->on_just_pressed_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(button_2d_node->on_just_released_cb != mp_const_none && button_2d_node->last_pressed == true && button_2d_node->pressed == false){
                        exec[0] = button_2d_node->on_just_released_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    // Save the state for tracking for callbacks
                    button_2d_node->last_pressed = button_2d_node->pressed;
                    button_2d_node->last_focused = button_2d_node->focused;

                    // After drawing everything, set pressed back to false.
                    // After this function is done looping through all the
                    // node callbacks, the gui tick is done again and it
                    // may be found that the button is still pressed but
                    // set to false anyways
                    button_2d_node->pressed = false;
                }
                break;
                case NODE_TYPE_GUI_BITMAP_BUTTON_2D:
                {
                    engine_gui_bitmap_button_2d_node_class_obj_t *bitmap_button_2d_node = node_base->node;
                    exec[1] = node_base->attr_accessor;

                    if(bitmap_button_2d_node->tick_cb != mp_const_none){
                        exec[0] = bitmap_button_2d_node->tick_cb;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_focused_cb != mp_const_none && bitmap_button_2d_node->focused == true){
                        exec[0] = bitmap_button_2d_node->on_focused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_just_focused_cb != mp_const_none && bitmap_button_2d_node->last_focused == false && bitmap_button_2d_node->focused == true){
                        exec[0] = bitmap_button_2d_node->on_just_focused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_just_unfocused_cb != mp_const_none && bitmap_button_2d_node->last_focused == true && bitmap_button_2d_node->focused == false){
                        exec[0] = bitmap_button_2d_node->on_just_unfocused_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_pressed_cb != mp_const_none && bitmap_button_2d_node->pressed == true){
                        exec[0] = bitmap_button_2d_node->on_pressed_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_just_pressed_cb != mp_const_none && bitmap_button_2d_node->last_pressed == false && bitmap_button_2d_node->pressed == true){
                        exec[0] = bitmap_button_2d_node->on_just_pressed_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    if(bitmap_button_2d_node->on_just_released_cb != mp_const_none && bitmap_button_2d_node->last_pressed == true && bitmap_button_2d_node->pressed == false){
                        exec[0] = bitmap_button_2d_node->on_just_released_cb;
                        mp_call_method_n_kw(0, 0, exec);
                    }

                    // Save the state for tracking for callbacks
                    bitmap_button_2d_node->last_pressed = bitmap_button_2d_node->pressed;
                    bitmap_button_2d_node->last_focused = bitmap_button_2d_node->focused;

                    // After drawing everything, set pressed back to false.
                    // After this function is done looping through all the
                    // node callbacks, the gui tick is done again and it
                    // may be found that the button is still pressed but
                    // set to false anyways
                    bitmap_button_2d_node->pressed = false;
                }
                break;
                case NODE_TYPE_PHYSICS_RECTANGLE_2D:
                {
                    engine_physics_node_base_t *physics_node_base = node_base->node;
                    if(physics_node_base->tick_cb != mp_const_none){
                        exec[0] = physics_node_base->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                case NODE_TYPE_PHYSICS_CIRCLE_2D:
                {
                    engine_physics_node_base_t *physics_node_base = node_base->node;
                    if(physics_node_base->tick_cb != mp_const_none){
                        exec[0] = physics_node_base->tick_cb;
                        exec[1] = node_base->attr_accessor;
                        exec[2] = mp_obj_new_float(dt_s);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
                break;
                default:
                    ENGINE_ERROR_PRINTF("This node type doesn't do anything? %d", node_base->type);
                break;
            }

            current_linked_list_node = current_linked_list_node->next;
        }
    }

    ENGINE_INFO_PRINTF("##### GAME TICKS COMPLETE #####\n");
}


void engine_invoke_all_node_draw_callbacks(){
    linked_list_node *current_linked_list_node = NULL;

    for(uint16_t ilx=0; ilx<engine_object_layer_count; ilx++){
        ENGINE_INFO_PRINTF("Starting drawing nodes in layer %d/%d", ilx, engine_object_layer_count-1);

        current_linked_list_node = engine_object_layers[ilx].start;

        while(current_linked_list_node != NULL){
            // Get the base node that every node is stored under
            engine_node_base_t *node_base = current_linked_list_node->object;

            switch(node_base->type){
                case NODE_TYPE_EMPTY:
                {
                    // Nothing but don't want to get to default case and error
                }
                break;
                case NODE_TYPE_CAMERA:
                {
                    // Nothing but don't want to get to default case and error
                }
                break;
                case NODE_TYPE_VOXELSPACE:
                {
                    engine_camera_draw_for_each(voxelspace_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_VOXELSPACE_SPRITE:
                {
                    engine_camera_draw_for_each(voxelspace_sprite_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_MESH_3D:
                {
                    engine_camera_draw_for_each(mesh_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_RECTANGLE_2D:
                {
                    engine_camera_draw_for_each(rectangle_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_LINE_2D:
                {
                    engine_camera_draw_for_each(line_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_CIRCLE_2D:
                {
                    engine_camera_draw_for_each(circle_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_SPRITE_2D:
                {
                    engine_camera_draw_for_each(sprite_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_TEXT_2D:
                {
                    engine_camera_draw_for_each(text_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_GUI_BUTTON_2D:
                {
                    engine_camera_draw_for_each(gui_button_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_GUI_BITMAP_BUTTON_2D:
                {
                    engine_camera_draw_for_each(gui_bitmap_button_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_PHYSICS_RECTANGLE_2D:
                {

                    engine_camera_draw_for_each(physics_rectangle_2d_node_class_draw, node_base);
                }
                break;
                case NODE_TYPE_PHYSICS_CIRCLE_2D:
                {
                    engine_camera_draw_for_each(physics_circle_2d_node_class_draw, node_base);
                }
                break;
                default:
                    ENGINE_ERROR_PRINTF("This node type doesn't do anything? %d", node_base->type);
                break;
            }

            current_linked_list_node = current_linked_list_node->next;
        }
    }

    ENGINE_INFO_PRINTF("##### GAME DRAWING COMPLETE #####\n");
}