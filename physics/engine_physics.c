#include "box2d/box2d.h"
#include "nodes/2D/physics_2d_node.h"


class ContactListener : public b2ContactListener{
    public:
        void BeginContact(b2Contact* contact){
            
        }
    
    private:
};


const float time_step = 1.f / 60.f;
const int32_t velocity_iterations = 4;
const int32_t position_iterations = 4;


b2World engine_physics_world({0.0f, -10.0f});
ContactListener engine_physics_contact_listener;
b2BodyDef engine_physics_dynamic_body_def;


extern "C"{
    void engine_physics_init(){
        engine_physics_world.SetContactListener(&engine_physics_contact_listener);
        engine_physics_dynamic_body_def.type = b2_dynamicBody;
    }


    void engine_physics_tick(){
        engine_physics_world.Step(time_step, velocity_iterations, position_iterations);
    }


    void engine_physics_create_body(engine_physics_2d_node_common_data_t *common_data){
        b2CircleShape *circle = new b2CircleShape();
        circle->m_radius = 5.0f;

        b2Body* body = engine_physics_world.CreateBody(&engine_physics_dynamic_body_def);
        body->CreateFixture(circle, 0.0f);
        body->SetTransform(b2Vec2(64.0f, 64.0f), 0.f);

        common_data->box2d_shape = circle;
        common_data->box2d_body = body;
    }
}