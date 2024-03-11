#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2d/physics_2d_node.h"
#include "math/vector2.h"
#include "math/engine_math.h"
#include "utility/engine_bit_collection.h"
#include "collision_shapes/polygon_collision_shape_2d.h"
#include "collision_contact_2d.h"
#include "py/obj.h"

#include <float.h>

// The maximum number of physics nodes that can exist in a scene is restricted
// by the number of IDs available to give those nodes. The number of IDs is
// limited so that IDs can be used with a simple paring function to index in
// a large bit array to check if collisions between objects have already occurred
#define PHYSICS_ID_MAX 180

// Bit array/collection to track nodes that have collided. In the `init` function
// this is sized so that the output indices from a simple paring function can fit
// (https://math.stackexchange.com/a/531914)
engine_bit_collection_t collided_physics_nodes;

// Buffer of available physics IDs. Filled with values
// 1 -> `PHYSICS_ID_MAX` on physics `init`. 
uint8_t available_physics_ids[PHYSICS_ID_MAX];
uint8_t next_available_physics_id = 0;


// Fill `available_physics_ids` with IDs from `1` -> `PHYSICS_ID_MAX`
// through indices `0` -> `PHYSICS_ID_MAX-1`
void engine_physics_init_ids(){
    for(uint8_t idx=0; idx<PHYSICS_ID_MAX; idx++){
        available_physics_ids[idx] = idx+1;
    }
}


uint8_t engine_physics_take_available_id(){
    if(next_available_physics_id >= PHYSICS_ID_MAX){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EnginePhysics: ERROR: Ran out of IDs to give physics nodes..."));
    }

    uint8_t id = available_physics_ids[next_available_physics_id];
    next_available_physics_id++;
    return id;
}


void engine_physics_give_back_id(uint8_t id){
    next_available_physics_id--;
    available_physics_ids[next_available_physics_id] = id;
}


// Using a simple pairing function (https://math.stackexchange.com/a/531914)
// calculate a unique number for the input IDs
uint32_t engine_physics_get_pair_index(uint8_t a_id, uint8_t b_id){
    return PHYSICS_ID_MAX * (uint32_t)a_id + (uint32_t)b_id;
}




linked_list engine_physics_nodes;
float engine_physics_gravity_x = 0.0f;
float engine_physics_gravity_y = -0.00981f;

float engine_physics_fps_limit_period_ms = 16.667f;
float engine_physics_fps_time_at_last_tick_ms = 0.0f;





bool engine_physics_check_collision(engine_node_base_t *physics_node_base_a, engine_node_base_t *physics_node_base_b, float *normal_x, float *normal_y, float *contact_x, float *contact_y){
    polygon_collision_shape_2d_class_obj_t *polygon_shape_a = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_collision_shape);
    polygon_collision_shape_2d_class_obj_t *polygon_shape_b = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_collision_shape);

    // Check that objects exist
    if(polygon_shape_a == MP_OBJ_NULL || polygon_shape_b == MP_OBJ_NULL){
        return false;
    }

    vector2_class_obj_t *node_a_position = mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_position);
    vector2_class_obj_t *node_b_position = mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_position);

    mp_obj_list_t *vertices_list_a = polygon_shape_a->vertices;
    mp_obj_list_t *vertices_list_b = polygon_shape_b->vertices;

    // Need to check each segment in polygon against
    // every other segment in the other polygon
    for(uint32_t ivax=0; ivax<vertices_list_a->len-1; ivax++){
        vector2_class_obj_t *line_a_start = (vector2_class_obj_t*)vertices_list_a->items[ivax];
        vector2_class_obj_t *line_a_end   = (vector2_class_obj_t*)vertices_list_a->items[ivax+1];

        for(uint32_t ivbx=0; ivbx<vertices_list_b->len-1; ivbx++){
            vector2_class_obj_t *line_b_start = (vector2_class_obj_t*)vertices_list_b->items[ivbx];
            vector2_class_obj_t *line_b_end   = (vector2_class_obj_t*)vertices_list_b->items[ivbx+1];

            if(engine_math_2d_do_segments_intersect(node_a_position->x + line_a_start->x,
                                                    node_a_position->y + line_a_start->y,
                                                    node_a_position->x + line_a_end->x,
                                                    node_a_position->y + line_a_end->y,

                                                    node_b_position->x + line_b_start->x,
                                                    node_b_position->y + line_b_start->y,
                                                    node_b_position->x + line_b_end->x,
                                                    node_b_position->y + line_b_end->y,
                                                    contact_x, contact_y)){
                return true;
            }
        }
    }


    return false;
}


void engine_physics_apply_impulses(){
    linked_list_node *physics_link_node = engine_physics_nodes.start;
    while(physics_link_node != NULL){
        engine_node_base_t *physics_node_base = physics_link_node->object;

        bool physics_node_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_dynamic));

        if(physics_node_dynamic){
            vector2_class_obj_t *physics_node_acceleration = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_acceleration);
            vector2_class_obj_t *physics_node_velocity = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_velocity);
            vector2_class_obj_t *physics_node_position = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_position);
            vector2_class_obj_t *physics_node_gravity_scale = mp_load_attr(physics_node_base->attr_accessor, MP_QSTR_gravity_scale);

            // Modifying these directly is good enough, don't need mp_store_attr even if using classes at main level!
            // Apply the user defined acceleration
            physics_node_velocity->x += physics_node_acceleration->x;
            physics_node_velocity->y += physics_node_acceleration->y;

            // Apply engine gravity (can be modifed by the user)
            physics_node_velocity->x -= engine_physics_gravity_x * physics_node_gravity_scale->x;
            physics_node_velocity->y -= engine_physics_gravity_y * physics_node_gravity_scale->y;

            // Apply velocity to the position
            physics_node_position->x += physics_node_velocity->x;
            physics_node_position->y += physics_node_velocity->y;
        }

        physics_link_node = physics_link_node->next;
    }
}


void engine_physics_init(){
    ENGINE_INFO_PRINTF("EnginePhysics: Starting...")
    engine_physics_init_ids();
    engine_bit_collection_create(&collided_physics_nodes, engine_physics_get_pair_index(PHYSICS_ID_MAX, PHYSICS_ID_MAX));
}


void engine_physics_tick(){
    // If it is not time to update physics, then don't, otherwise track when physics was updated last
    if(millis() - engine_physics_fps_time_at_last_tick_ms < engine_physics_fps_limit_period_ms){
        return;
    }else{
        engine_physics_fps_time_at_last_tick_ms = millis();
    }

    // Loop through all nodes and test for collision against
    // all other nodes (not optimized checking of if nodes are
    // even possibly close to each other)
    linked_list_node *physics_link_node_a = engine_physics_nodes.start;
    while(physics_link_node_a != NULL){
        // Now check 'a' against all nodes 'b'
        linked_list_node *physics_link_node_b = engine_physics_nodes.start;

        while(physics_link_node_b != NULL){
            // Make sure we are not checking against ourselves
            if(physics_link_node_a->object != physics_link_node_b->object){
                engine_node_base_t *physics_node_base_a = physics_link_node_a->object;
                engine_node_base_t *physics_node_base_b = physics_link_node_b->object;

                bool physics_node_a_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_a->attr_accessor, MP_QSTR_dynamic));
                bool physics_node_b_dynamic = mp_obj_get_int(mp_load_attr(physics_node_base_b->attr_accessor, MP_QSTR_dynamic));

                // Only check collision if at atleast one of the involved nodes
                // is dynamic (do not want static nodes generating collisions)
                if(physics_node_a_dynamic == true || physics_node_b_dynamic == true){
                    engine_physics_2d_node_common_data_t *physics_node_common_data_a = physics_node_base_a->node_common_data;
                    engine_physics_2d_node_common_data_t *physics_node_common_data_b = physics_node_base_b->node_common_data;

                    // To consistently generate the index for `already_collided`
                    // the ID pair needs to be sorted consistently (don't matter
                    // how, just as long as it is consistent)
                    uint8_t a_id = physics_node_common_data_a->physics_id;
                    uint8_t b_id = physics_node_common_data_b->physics_id;

                    // Sort so that id_max*a_id + b_id comes out the same
                    // no matter which node is `a` or `b`
                    if(a_id > b_id){
                        b_id = physics_node_common_data_a->physics_id;
                        a_id = physics_node_common_data_b->physics_id;
                    }

                    // Get the perfect hash code (can be perfect since the range is limited)
                    uint32_t pair_index = engine_physics_get_pair_index(a_id, b_id);
                    bool already_collided = engine_bit_collection_get(&collided_physics_nodes, pair_index);

                    // Skip collision check if already collied
                    // before, otherwise set that these have collied
                    // this frame already
                    if(already_collided){
                        physics_link_node_b = physics_link_node_b->next;
                        continue;
                    }else{
                        engine_bit_collection_set(&collided_physics_nodes, pair_index);
                    }

                    float normal_x = 0.0f;
                    float normal_y = 0.0f;
                    float contact_x = 0.0f;
                    float contact_y = 0.0f;

                    if(engine_physics_check_collision(physics_node_base_a, physics_node_base_b, &normal_x, &normal_y, &contact_x, &contact_y)){
                        mp_obj_t collision_contact_data[5];
                        collision_contact_data[0] = mp_obj_new_float(contact_x);
                        collision_contact_data[1] = mp_obj_new_float(contact_y);
                        collision_contact_data[2] = mp_obj_new_float(0.0f);
                        collision_contact_data[3] = mp_obj_new_float(0.0f);

                        mp_obj_t exec[3];

                        // Call A callback
                        collision_contact_data[4] = physics_link_node_b->object;
                        exec[0] = physics_node_common_data_a->collision_cb;
                        exec[1] = physics_node_base_a->attr_accessor;
                        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                        mp_call_method_n_kw(1, 0, exec);

                        // Call B callback
                        collision_contact_data[4] = physics_link_node_a->object;
                        exec[0] = physics_node_common_data_b->collision_cb;
                        exec[1] = physics_node_base_b->attr_accessor;
                        exec[2] = collision_contact_2d_class_new(&collision_contact_2d_class_type, 5, 0, collision_contact_data);
                        mp_call_method_n_kw(1, 0, exec);
                    }
                }
            }
            physics_link_node_b = physics_link_node_b->next;
        }
        physics_link_node_a = physics_link_node_a->next;
    }

    // ENGINE_FORCE_PRINTF(" ");
    engine_physics_apply_impulses();

    // After everything physics related is done, reset the bit array
    // used for tracking which pairs of nodes had already collided
    engine_bit_collection_erase(&collided_physics_nodes);
}


linked_list_node *engine_physics_track_node(engine_node_base_t *obj){
    ENGINE_INFO_PRINTF("Tracking physics node %p", obj);
    return linked_list_add_obj(&engine_physics_nodes, obj);
}


void engine_physics_untrack_node(linked_list_node *physics_list_node){
    ENGINE_INFO_PRINTF("Untracking physics node");
    linked_list_del_list_node(&engine_physics_nodes, physics_list_node);
}


void engine_physics_clear_all(){
    ENGINE_INFO_PRINTF("Untracking physics nodes...");
    linked_list_clear(&engine_physics_nodes);
}