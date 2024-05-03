#include "engine_gui.h"
#include "input/engine_input_common.h"
#include "input/engine_input_module.h"
#include "nodes/2D/gui_button_2d_node.h"
#include "nodes/2D/gui_bitmap_button_2d_node.h"
#include "math/engine_math.h"
#include <float.h>


linked_list engine_guis;
engine_node_base_t *focused_gui_node_base = NULL;


bool gui_focused = false;


linked_list_node *engine_gui_track(engine_node_base_t *obj){
    ENGINE_INFO_PRINTF("Tracking new gui node");
    return linked_list_add_obj(&engine_guis, obj);
}


void engine_gui_untrack(linked_list_node *gui_list_node){
    ENGINE_INFO_PRINTF("Untracking gui node");
    linked_list_del_list_node(&engine_guis, gui_list_node);
}


void engine_gui_clear_all(){
    ENGINE_INFO_PRINTF("Untracking all gui nodes...");
    
    linked_list_node *current = engine_guis.start;
    while(current != NULL){
        m_del_obj((mp_obj_type_t*)((mp_obj_base_t*)current->object)->type, current->object);

        current = current->next;
    }
}


void engine_gui_reset(){
    focused_gui_node_base = NULL;
    gui_focused = false;
    engine_input_reset_gui_toggle_button();
}


bool engine_gui_get_focus(){
    return gui_focused;
}


void engine_gui_focus_node(engine_node_base_t *gui_node_base){
    // Focus this node
    if(mp_obj_is_type(gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
        engine_gui_bitmap_button_2d_node_class_obj_t *gui_node = gui_node_base->node;
        gui_node->focused = true;
    }else{
        engine_gui_button_2d_node_class_obj_t *gui_node = gui_node_base->node;
        gui_node->focused = true;
    }

    // Unfocus the last one
    if(focused_gui_node_base != NULL){
        if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
            engine_gui_bitmap_button_2d_node_class_obj_t *gui_node = focused_gui_node_base->node;
            gui_node->focused = false;
        }else{
            engine_gui_button_2d_node_class_obj_t *gui_node = focused_gui_node_base->node;
            gui_node->focused = false;
        }
    }

    focused_gui_node_base = gui_node_base;
}


void engine_gui_toggle_focus(){
    gui_focused = !gui_focused;

    // If just focused, loop through all elements and focus
    // the first node if no other node is already focused
    if(gui_focused && engine_guis.start != NULL){
        linked_list_node *current_gui_list_node = engine_guis.start;

        while(current_gui_list_node != NULL){
            engine_node_base_t *gui_node_base = current_gui_list_node->object;

            bool focused = false;

            if(mp_obj_is_type(gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
                focused = ((engine_gui_bitmap_button_2d_node_class_obj_t*)gui_node_base->node)->focused;
            }else{
                focused = ((engine_gui_button_2d_node_class_obj_t*)gui_node_base->node)->focused;
            }
            
            // Don't need to focus any nodes, this one already is focused, end function
            if(focused){
                return;
            }

            current_gui_list_node = current_gui_list_node->next;
        }

        // Made it this far, must mean that no gui nodes are focused, focus the first one
        engine_gui_focus_node(engine_guis.start->object);
    }
}


engine_node_base_t *engine_gui_get_focused(){
    return focused_gui_node_base;
}


bool engine_gui_is_left_check(float angle_radians){
    // 135 to 225
    if(angle_radians >= 135.0f && angle_radians <= 225.0f){
        return true;
    }else{
        return false;
    }
}


bool engine_gui_is_right_check(float angle_radians){
    // 315 to 45
    if((angle_radians >= 315.0f && angle_radians <= 360.0f) || (angle_radians >= 0.0f && angle_radians <= 45.0f)){
        return true;
    }else{
        return false;
    }
}


bool engine_gui_is_up_check(float angle_radians){
    // 225 to 315 (remember positions towards the top of the screen are at -y)
    if(angle_radians >= 225.0f && angle_radians <= 315.0f){
        return true;
    }else{
        return false;
    }
}


bool engine_gui_is_down_check(float angle_radians){
    // 45 to 135
    if(angle_radians >= 45.0f && angle_radians <= 135.0f){
        return true;
    }else{
        return false;
    }
}


// Given `focused_gui_node_base`, find the next closest
// gui node.
void engine_gui_select_closest(bool (direction_check)(float)){
    // Make sure to not do anything if no GUI nodes in scene
    if(focused_gui_node_base == NULL){
        return;
    }

    // Get the position of the currently focused GUI node
    vector2_class_obj_t *focused_gui_position = NULL;
    if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
        focused_gui_position = ((engine_gui_bitmap_button_2d_node_class_obj_t*)focused_gui_node_base->node)->position;
    }else{
        focused_gui_position = ((engine_gui_button_2d_node_class_obj_t*)focused_gui_node_base->node)->position;
    }

    // Setup for looping through all GUI nodes and finding closest
    linked_list_node *current_gui_list_node = engine_guis.start;
    engine_node_base_t *closest_gui_node_base = NULL;
    float shortest_distance = FLT_MAX;

    while(current_gui_list_node != NULL){

        // If we're looping through all the gui nodes and
        // come across the already focused node, skip it
        if(current_gui_list_node->object == focused_gui_node_base){
            current_gui_list_node = current_gui_list_node->next;
            continue;
        }

        // Get the node base of the currently looping
        // through node
        engine_node_base_t *searching_gui_node_base = current_gui_list_node->object;

        // Get the position of the currently focused
        // GUI node
        vector2_class_obj_t *searching_gui_position = NULL;
        if(mp_obj_is_type(searching_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
            searching_gui_position = ((engine_gui_bitmap_button_2d_node_class_obj_t*)searching_gui_node_base->node)->position;
        }else{
            searching_gui_position = ((engine_gui_button_2d_node_class_obj_t*)searching_gui_node_base->node)->position;
        }

        // Get the angle between the focused node and
        // the node we looped to now
        float angle_radians = engine_math_angle_between(focused_gui_position->x.value, focused_gui_position->y.value, searching_gui_position->x.value, searching_gui_position->y.value);

        // Convert from -180 ~ 180 to 0 ~ 360: https://stackoverflow.com/a/25725005
        angle_radians = fmodf(angle_radians + TWICE_PI, TWICE_PI);

        // Convert to degrees (for readability)
        angle_radians = angle_radians * 180.0f / PI;

        // Using the passed function, see if the angle
        // is in the direction we want to go
        if(direction_check(angle_radians)){
            float distance = engine_math_distance_between(focused_gui_position->x.value, focused_gui_position->y.value, searching_gui_position->x.value, searching_gui_position->y.value);

            // If the distance is closer than the last one
            // we compared to, set it as the closest
            if(distance < shortest_distance){
                shortest_distance = distance;
                closest_gui_node_base = searching_gui_node_base;
            }
        }

        current_gui_list_node = current_gui_list_node->next;
    }

    // Found one! Focus it and make sure to unfocus the
    // previously focused node
    if(closest_gui_node_base != NULL){
        engine_gui_focus_node(closest_gui_node_base);
    }
}


void engine_gui_tick(){
    // Every tick, see if the button to toggle GUI focus was pressed.
    // If the GUI toggle button is 0 that means None was set for the
    // toggle button and that we should not automaticaly switch focus
    uint16_t gui_toggle_button = engine_input_get_gui_toggle_button();
    if(gui_toggle_button != 0 && check_just_pressed(gui_toggle_button)){
        engine_gui_toggle_focus();

        // If unfocus GUI entirely, make sure the focused node gets unfocused
        if(focused_gui_node_base != NULL && gui_focused == false){
            engine_gui_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;

            if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
                ((engine_gui_bitmap_button_2d_node_class_obj_t*)focused_gui_node_base->node)->focused = false;
            }else{
                ((engine_gui_button_2d_node_class_obj_t*)focused_gui_node_base->node)->focused = false;
            }

            focused_gui_node_base = NULL;
        }
    }

    // Only run the GUI selection logic when the
    // gui is focused due to the input module
    if(gui_focused == false){
        return;
    }

    if(check_just_pressed(BUTTON_DPAD_LEFT)){
        engine_gui_select_closest(engine_gui_is_left_check);
    }else if(check_just_pressed(BUTTON_DPAD_RIGHT)){
        engine_gui_select_closest(engine_gui_is_right_check);
    }else if(check_just_pressed(BUTTON_DPAD_UP)){
        engine_gui_select_closest(engine_gui_is_up_check);
    }else if(check_just_pressed(BUTTON_DPAD_DOWN)){
        engine_gui_select_closest(engine_gui_is_down_check);
    }

    // Check if the focused/highlighted node should respond
    // to the currently pressed hardware button
    if(focused_gui_node_base != NULL){
        uint16_t button = 0;

        // Figure out what hardware button this button should respond to
        if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
            engine_gui_bitmap_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
            button = focused_node->button;
        }else{
            engine_gui_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
            button = focused_node->button;
        }
        
        // Check if the button is pressed, if it is, indicate with flag that it is pressed
        if(check_pressed(button)){
            if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
                engine_gui_bitmap_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
                focused_node->pressed = true;
            }else{
                engine_gui_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
                focused_node->pressed = true;
            }
        }
    }   
}