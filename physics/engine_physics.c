#include "engine_physics.h"
#include "debug/debug_print.h"
#include "nodes/2D/physics_2d_node.h"
#include "math/vector2.h"


#define PHYSAC_STANDALONE
#define PHYSAC_NO_THREADS
#define PHYSAC_IMPLEMENTATION
#include "libs/physac/physac.h"

#define     PI                     3.14159265358979323846
#define     RAD2DEG                (180.0f/PI)
#define     DEG2RAD                (PI/180.0f)


// A linked list of physics nodes to loop
// through to copy all parameters quickly
linked_list engine_physics_nodes;


void engine_physics_init(){
    // Initialize physics and default physics bodies
    InitPhysics();
    SetPhysicsGravity(0.0f, 0.01f);
}


void engine_physics_tick(){
    ENGINE_INFO_PRINTF("Physics: running physics step...");
    RunPhysicsStep();
}


void engine_physics_create_rectangle_body(engine_physics_2d_node_common_data_t *common_data){
    common_data->physac_body = CreatePhysicsBodyRectangle((Vector2){0.0f, 0.0f}, 15.0f, 5.0f, 1);
}


void engine_physics_get_body_xy(void *body, double *x, double *y){
    PhysicsBody physics_body = body;
    *x = physics_body->position.x;
    *y = physics_body->position.y;
}


linked_list_node *engine_physics_track_node(engine_node_base_t *obj){
    ENGINE_INFO_PRINTF("Tracking physics node");
    return linked_list_add_obj(&engine_physics_nodes, obj);
}


void engine_physics_untrack_node(linked_list_node *physics_list_node){
    ENGINE_INFO_PRINTF("Untracking physics node");
    linked_list_del_list_node(&engine_physics_nodes, physics_list_node);
}


void engine_physics_sync_engine_nodes_to_bodies(){
    linked_list_node *current_linked_list_node = engine_physics_nodes.start;

    while(current_linked_list_node != NULL){
        ENGINE_INFO_PRINTF("Physics: syncing engine node to physics body...");

        engine_node_base_t *node_base = current_linked_list_node->object;
        engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;
        vector2_class_obj_t *engine_node_postion;
        float rotation_degrees = 0.0f;
        bool dynamic = false;
        
        if(node_base->inherited){
            engine_node_postion = mp_load_attr(node_base->node, MP_QSTR_position);
            rotation_degrees = mp_obj_get_float(mp_load_attr(node_base->node, MP_QSTR_rotation));
            dynamic = mp_obj_get_int(mp_load_attr(node_base->node, MP_QSTR_dynamic));
        }else{
            engine_node_postion = mp_load_attr(node_base, MP_QSTR_position);
            rotation_degrees = mp_obj_get_float(mp_load_attr(node_base, MP_QSTR_rotation));
            dynamic = mp_obj_get_int(mp_load_attr(node_base, MP_QSTR_dynamic));
        }
        
        PhysicsBody body = common_data->physac_body;
        body->position.x = engine_node_postion->x;
        body->position.y = engine_node_postion->y;
        body->orient = rotation_degrees * DEG2RAD;
        body->enabled = dynamic;

        current_linked_list_node = current_linked_list_node->next;
    }
}


void engine_physics_sync_bodies_to_engine_nodes(){
    linked_list_node *current_linked_list_node = engine_physics_nodes.start;

    while(current_linked_list_node != NULL){
        ENGINE_INFO_PRINTF("Physics: syncing physics bodies to engine nodes...");

        engine_node_base_t *node_base = current_linked_list_node->object;
        engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;
        vector2_class_obj_t *engine_node_position;
        
        PhysicsBody body = common_data->physac_body;

        if(node_base->inherited){
            engine_node_position = mp_load_attr(node_base->node, MP_QSTR_position);
            mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(body->orient * RAD2DEG));
        }else{
            engine_node_position = mp_load_attr(node_base, MP_QSTR_position);
            mp_store_attr(node_base, MP_QSTR_rotation, mp_obj_new_float(body->orient * RAD2DEG));
        }

        engine_node_position->x = body->position.x;
        engine_node_position->y = body->position.y;

        current_linked_list_node = current_linked_list_node->next;
    }
}