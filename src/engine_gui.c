#include "engine_gui.h"
#include "io/engine_io_buttons.h"
#include "io/engine_io_module.h"
#include "nodes/2D/gui_button_2d_node.h"
#include "nodes/2D/gui_bitmap_button_2d_node.h"
#include "math/engine_math.h"
#include "engine_collections.h"
#include <float.h>


engine_node_base_t *focused_gui_node_base = NULL;


bool gui_focused = false;

// Whether wrapping around when navigating GUI elements is enabled.
bool gui_wrapping_enabled = true;

// Whether passing inputs to game is enabled
bool gui_passing_enabled = false;


void resolve_gui_node_position(engine_node_base_t *gui_node_base, float *x, float *y){
    // engine_inheritable_2d_t inherited;
    // node_base_inherit_2d(gui_node_base, &inherited);
    //
    // *x = inherited.px;
    // *y = inherited.py;
    node_base_get_child_absolute_xy(x, y, NULL, NULL, gui_node_base);
}


bool resolve_gui_node_is_disabled(engine_node_base_t *gui_node_base){
    if(mp_obj_is_type(gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
        return (bool)mp_obj_get_int(((engine_gui_bitmap_button_2d_node_class_obj_t*)gui_node_base->node)->disabled);
    }else{
        return (bool)mp_obj_get_int(((engine_gui_button_2d_node_class_obj_t*)gui_node_base->node)->disabled);
    }
}


bool resolve_gui_node_before_focused(engine_node_base_t *gui_node_base){
    mp_obj_t exec[2];
    exec[1] = gui_node_base->attr_accessor;

    if(mp_obj_is_type(gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
        exec[0] = ((engine_gui_bitmap_button_2d_node_class_obj_t*)gui_node_base->node)->on_before_focused_cb;
    }else{
        exec[0] = ((engine_gui_button_2d_node_class_obj_t*)gui_node_base->node)->on_before_focused_cb;
    }

    mp_obj_t result = mp_call_method_n_kw(0, 0, exec);

    // Check that the result is bool and if it is false, do not focus
    // this and search for a different button to focus
    if(mp_obj_is_bool(result) == true){
        return mp_obj_get_int(result);
    }else{
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineGUI: ERROR: `on_before_focused` should return bool, got %s"), mp_obj_get_type_str(result));
    }
}


void engine_gui_reset(){
    focused_gui_node_base = NULL;
    gui_focused = false;
    gui_wrapping_enabled = true;
    gui_passing_enabled = false;
}


bool engine_gui_is_gui_focused(){
    return gui_focused;
}


void engine_gui_focus_node(engine_node_base_t *gui_node_base){
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

    // Focus this node
    if(mp_obj_is_type(gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
        engine_gui_bitmap_button_2d_node_class_obj_t *gui_node = gui_node_base->node;
        gui_node->focused = true;
    }else{
        engine_gui_button_2d_node_class_obj_t *gui_node = gui_node_base->node;
        gui_node->focused = true;
    }

    focused_gui_node_base = gui_node_base;
}


bool engine_gui_toggle_focus(){
    return engine_gui_set_focused(!gui_focused);
}


bool engine_gui_set_focused(bool focus_gui){
    if(focus_gui != gui_focused){
        gui_focused = focus_gui;

        // If just focused, loop through all elements and focus
        // the first node if no other node is already focused
        linked_list *gui_list = engine_collections_get_gui_list();

        if(gui_focused && gui_list->start != NULL){
            linked_list_node *current_gui_list_node = gui_list->start;

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
                    return gui_focused;
                }

                current_gui_list_node = current_gui_list_node->next;
            }

            // Made it this far, must mean that no gui nodes are focused, focus the first one
            engine_gui_focus_node(gui_list->start->object);
        }
    }
    return gui_focused;
}


engine_node_base_t *engine_gui_get_focused(){
    return focused_gui_node_base;
}


void engine_gui_set_wrapping(bool enabled){
    gui_wrapping_enabled = enabled;
}

bool engine_gui_get_wrapping(){
    return gui_wrapping_enabled;
}


void engine_gui_set_passing(bool enabled){
    gui_passing_enabled = enabled;
}

bool engine_gui_get_passing(){
    return gui_passing_enabled;
}


// Given `focused_gui_node_base` and a direction, find the next closest gui node.
void engine_gui_select_closest(float dir_x, float dir_y, bool allow_wrap){
    // Make sure to not do anything if no GUI nodes in scene
    if(focused_gui_node_base == NULL){
        return;
    }

    float dir_len_sqr = engine_math_vector_length_sqr(dir_x, dir_y);

    // Get the position of the currently focused GUI node
    float focused_gui_position_x = 0.0f;
    float focused_gui_position_y = 0.0f;
    resolve_gui_node_position(focused_gui_node_base, &focused_gui_position_x, &focused_gui_position_y);

    // Setup for looping through all GUI nodes and finding closest
    linked_list *gui_list = engine_collections_get_gui_list();
    if(gui_list->count < 2){
        return;
    }
    engine_node_base_t *closest_gui_node_base = NULL;
    linked_list_node *current_gui_list_node = gui_list->start;
    float best_node_value = FLT_MAX;

    while(current_gui_list_node != NULL){

        // If we're looping through all the gui nodes and
        // come across the already focused node, skip it
        if(current_gui_list_node->object == focused_gui_node_base){
            current_gui_list_node = current_gui_list_node->next;
            continue;
        }

        // Get the node base of the currently looping through node
        engine_node_base_t *searching_gui_node_base = current_gui_list_node->object;

        // If the node we're looking at is disabled, do not try to focus it
        if(resolve_gui_node_is_disabled(searching_gui_node_base)){
            current_gui_list_node = current_gui_list_node->next;
            continue;
        }

        // If the button we're looking at focusing doesn't allow it,
        // skip and keep looking for other buttons that do
        if(!resolve_gui_node_before_focused(searching_gui_node_base)){
            current_gui_list_node = current_gui_list_node->next;
            continue;
        }

        // Get the position of the current GUI node
        float searching_gui_position_x = 0.0f;
        float searching_gui_position_y = 0.0f;
        resolve_gui_node_position(searching_gui_node_base, &searching_gui_position_x, &searching_gui_position_y);

        // Find the position of the current node relative to the focused node, as a vector.
        float rel_pos_x = searching_gui_position_x - focused_gui_position_x;
        float rel_pos_y = searching_gui_position_y - focused_gui_position_y;
        float rel_pos_len_sqr = engine_math_vector_length_sqr(rel_pos_x, rel_pos_y);

        float dir_dot_rel = engine_math_dot_product(dir_x, dir_y, rel_pos_x, rel_pos_y);
        // Skip any nodes "behind" the focused node.
        if (dir_dot_rel >= 0.0f){
            // Find the angle between the position vector and the dir, using the dot product property:
            //   a·b = |a||b|cos(theta)
            float cos_theta_sqr = dir_dot_rel * dir_dot_rel / (dir_len_sqr * rel_pos_len_sqr);

            // Accept only nodes within the 90 degree cone in front of the focused node, so cos(theta) >= cos(45deg) = sqrt(2)/2,
            // so cos(theta)^2 >= 0.5.
            if(cos_theta_sqr >= 0.5f){
                // Prefer nodes that are closer to the focused node, and at a direction closer to the specified direction.
                //
                // In particular, consider a rectangular grid of nodes, where A is the focused node. The geometric distance
                // |AC| is 1, and |AB| is some constant k. The dir is a diagonal vector pointing down and right, at 45 degrees.
                // We expect that in a square or close to square grid (i.e. when k is close to 1), node D will be selected.
                // We also never want the node F (which has theta closer to 0) to be selected.
                //
                //     [A]  B
                //      C   D
                //      E   F
                //
                //                                                           dist1 =                 dist2 =                 dist3 =
                // Node      distance        |theta|       cos(theta)  distance/cos(theta)^2   distance/cos(theta)^3   distance/cos(theta)^4
                // -------------------------------------------------------------------------------------------------------------------------
                //   B          k             45deg        sqrt(2)/2           2k                     2.83k                    4k
                //   C          1             45deg        sqrt(2)/2           2                      2.83                     4
                //   D      sqrt(k^2+1)   atan(k)-45deg
                //   F      sqrt(k^2+4)  atan(k/2)-45deg
                //
                // For k=1:
                //   D         1.41             0              1              1.41                    1.41                    1.41
                //   F         2.24           -18deg         0.95             2.48                    2.62                    2.76
                //                                                          best is D               best is D               best is D
                // For k=1.5:
                //   D         1.80            11deg         0.98             1.87                    1.91                    1.95
                //   F         2.50            -8deg         0.99             2.55                    2.58                    2.60
                //                                                          best is D               best is D               best is D
                // For k=2:
                //   D         2.23            18deg         0.94             2.48                    2.62                    2.76
                //   F         2.83             0              1              2.83                    2.83                    2.83
                //                                                          best is C!              best is D               best is D
                // For k=2.5:
                //   D         2.69            23deg         0.92             3.19                    3.47                    3.77
                //   F         3.20             6deg         0.99             3.24                    3.26                    3.28
                //                                                          best is C!              best is C!               best is F!
                //
                // The last three columns present three candidates for the distance metric function.
                // The node with the lowest value is selected. The dist2 formula will select the node D
                // even for k=2, while the other formulas will select C or F instead. The dist2 formula
                // will also never select node F, regardless of the value of k, which makes it a good candidate.
                //
                // To avoid using the square root function, we use the square of the above formula, so finally
                // node_value = dist2^2 = distance^2 / cos(theta)^6.
                float node_value = rel_pos_len_sqr / (cos_theta_sqr * cos_theta_sqr * cos_theta_sqr);
                if(node_value < best_node_value){
                    best_node_value = node_value;
                    closest_gui_node_base = searching_gui_node_base;
                }
            }
        }

        current_gui_list_node = current_gui_list_node->next;
    }

    if(allow_wrap && closest_gui_node_base == NULL){
        // Nothing found in the specified direction, so try wrapping around. This is a rare case, so redoing some calculations
        // from before is fine. We don't optimize this case as much as the more common case above.

        current_gui_list_node = gui_list->start;
        while(current_gui_list_node != NULL){
            // Initial logic same as above.
            if(current_gui_list_node->object == focused_gui_node_base){
                current_gui_list_node = current_gui_list_node->next;
                continue;
            }
            engine_node_base_t *searching_gui_node_base = current_gui_list_node->object;
            float searching_gui_position_x = 0.0f;
            float searching_gui_position_y = 0.0f;
            resolve_gui_node_position(searching_gui_node_base, &searching_gui_position_x, &searching_gui_position_y);

            // If the node we're looking at is disabled, do not try to focus it
            if(resolve_gui_node_is_disabled(searching_gui_node_base)){
                current_gui_list_node = current_gui_list_node->next;
                continue;
            }

            float rel_pos_x = searching_gui_position_x - focused_gui_position_x;
            float rel_pos_y = searching_gui_position_y - focused_gui_position_y;
            float rel_pos_len_sqr = engine_math_vector_length_sqr(rel_pos_x, rel_pos_y);
            float dir_dot_rel = engine_math_dot_product(dir_x, dir_y, rel_pos_x, rel_pos_y);
            // Only analyze nodes "behind" the focused node.
            if (dir_dot_rel <= 0.0f){
                float cos_theta_sqr = dir_dot_rel * dir_dot_rel / (dir_len_sqr * rel_pos_len_sqr);
                // Accept only nodes within the 90 degree cone behind the focused node, so cos(theta) <= cos(135deg) = -sqrt(2)/2,
                // so cos(theta)^2 >= 0.5.
                if(cos_theta_sqr >= 0.5f){
                    // When wrapping, the furthest node in the opposite direction along the dir axis should
                    // be selected. The exact formula is probably not that important, as most of the time
                    // wrapping will happen with dir having only one non-zero component. Diagonal wrapping
                    // is also possible, but it is difficult to even determine what is the expected result.
                    float node_value = -rel_pos_len_sqr * cos_theta_sqr * cos_theta_sqr * cos_theta_sqr;
                    if(node_value < best_node_value){
                        best_node_value = node_value;
                        closest_gui_node_base = searching_gui_node_base;
                    }
                }
            }

            current_gui_list_node = current_gui_list_node->next;
        }
    }

    // Found one! Focus it and make sure to unfocus the
    // previously focused node
    if(closest_gui_node_base != NULL){
        engine_gui_focus_node(closest_gui_node_base);
    }
}


void engine_gui_clear_focused(){
    // If the GUI layer is focused, find a new node when
    // the current focused node is to be cleared (likely gc'ed)
    if(gui_focused){
        linked_list *gui_list = engine_collections_get_gui_list();
        linked_list_node *current_gui_list_node = gui_list->start;

        // Get the position of the currently focused GUI node
        float focused_gui_position_x = 0.0f;
        float focused_gui_position_y = 0.0f;
        resolve_gui_node_position(focused_gui_node_base, &focused_gui_position_x, &focused_gui_position_y);

        engine_node_base_t *closest_gui_node_base = NULL;
        float shortest_distance = FLT_MAX;

        while(current_gui_list_node != NULL){
            engine_node_base_t *searching_gui_node_base = current_gui_list_node->object;
            float searching_gui_position_x = 0.0f;
            float searching_gui_position_y = 0.0f;
            resolve_gui_node_position(searching_gui_node_base, &searching_gui_position_x, &searching_gui_position_y);

            float distance = engine_math_distance_between(focused_gui_position_x, focused_gui_position_y, searching_gui_position_x, searching_gui_position_y);

            // If the distance is closer than the last one
            // we compared to, set it as the closest. Make
            // sure not comparing focused vs. focused
            if(distance < shortest_distance && focused_gui_node_base != searching_gui_node_base){
                shortest_distance = distance;
                closest_gui_node_base = searching_gui_node_base;
            }

            current_gui_list_node = current_gui_list_node->next;
        }

        // Check if we found an alternative, node, focus it if we did
        if(closest_gui_node_base != NULL){
            engine_gui_focus_node(closest_gui_node_base);
        }else{
            focused_gui_node_base = NULL;
        }
    }else{
        focused_gui_node_base = NULL;
    }
}


void engine_gui_tick(){
    // Only run the GUI selection logic when the
    // gui is focused due to the io module
    if(gui_focused == false){
        return;
    }

    float dir_x = 0.0f;
    float dir_y = 0.0f;
    if(button_is_pressed_autorepeat(&BUTTON_DPAD_LEFT)){
        dir_x = -1.0f;
    }else if(button_is_pressed_autorepeat(&BUTTON_DPAD_RIGHT)){
        dir_x = 1.0f;
    }

    if(button_is_pressed_autorepeat(&BUTTON_DPAD_UP)){
        dir_y = -1.0f;
    }else if(button_is_pressed_autorepeat(&BUTTON_DPAD_DOWN)){
        dir_y = 1.0f;
    }

    if((bool)dir_x || (bool)dir_y){
        bool allow_wrap = false;
        if(gui_wrapping_enabled){
            // Allow wrapping only if any of the dpad buttons was just pressed.
            // Use binary arithmetic instead of method calls for speed.
            allow_wrap = (
                (BUTTON_CODE_DPAD_UP | BUTTON_CODE_DPAD_DOWN | BUTTON_CODE_DPAD_LEFT | BUTTON_CODE_DPAD_RIGHT) &
                pressed_buttons & ~prev_pressed_buttons) != 0;
        }
        engine_gui_select_closest(dir_x, dir_y, allow_wrap);
    }

    // Before checking if the focused node should be pressed,
    // make sure it is not disabled, if it is, to not check
    if(resolve_gui_node_is_disabled(focused_gui_node_base)){
        return;
    }

    // Check if the focused/highlighted node should respond
    // to the currently pressed hardware button
    if(focused_gui_node_base != NULL){
        button_class_obj_t* button = NULL;

        // Figure out what hardware button this button should respond to
        if(mp_obj_is_type(focused_gui_node_base, &engine_gui_bitmap_button_2d_node_class_type)){
            engine_gui_bitmap_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
            button = focused_node->button;
        }else{
            engine_gui_button_2d_node_class_obj_t *focused_node = focused_gui_node_base->node;
            button = focused_node->button;
        }

        // Check if the button is pressed, if it is, indicate with flag that it is pressed
        if(button_is_pressed(button)){
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
