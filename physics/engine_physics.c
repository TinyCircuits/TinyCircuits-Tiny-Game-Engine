#include "engine_physics.h"
#include "debug/debug_print.h"


#define PHYSAC_STANDALONE
#define PHYSAC_NO_THREADS
#define PHYSAC_IMPLEMENTATION
#include "libs/physac/physac.h"


void engine_physics_init(){
    // Initialize physics and default physics bodies
    InitPhysics();
}


void engine_physics_tick(){
    ENGINE_INFO_PRINTF("Physics: running physics step...");
    RunPhysicsStep();
}


void engine_physics_create_rectangle_body(engine_physics_2d_node_common_data_t *common_data){
    PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){0, 0}, 25, 25, 1);

    // b2CircleShape *circle = new b2CircleShape();
    // circle->m_radius = 5.0f;

    // b2Body* body = engine_physics_world.CreateBody(&engine_physics_dynamic_body_def);
    // body->CreateFixture(circle, 0.0f);
    // body->SetTransform(b2Vec2(64.0f, 64.0f), 0.f);

    // common_data->box2d_shape = circle;
    // common_data->box2d_body = body;
}