#ifndef PHYSICS_NODE_BASE_H
#define PHYSICS_NODE_BASE_H

#include "py/obj.h"
#include "node_base.h"

typedef struct{
    mp_obj_t position;                      // Vector2: 2d xy position of this node
    
    mp_obj_t velocity;                      // Vector2 (Absolute velocity)
    float angular_velocity;

    float rotation;                      // float (Current rotation angle)

    mp_obj_t density;                       // How dense the node is

    mp_obj_t friction;

    mp_obj_t bounciness;                    // Restitution or elasticity

    mp_obj_t dynamic;                       // Flag indicating if node is dynamic and moving around due to physics or static
    mp_obj_t solid;                         // May want collision callbacks to happen without impeding objects, set to false

    mp_obj_t gravity_scale;                 // Vector2 allowing scaling affects of gravity. Set to 0,0 for no gravity

    mp_obj_t outline;
    mp_obj_t outline_color;

    int16_t collision_layer;
    bool was_colliding; // Used for calling `on_separate_cb` internally
    bool colliding;     // Used internally and exposed to users

    uint8_t physics_id;

    float mass;
    float inverse_mass;
    float inverse_moment_of_inertia;        // https://www.concepts-of-physics.com/mechanics/moment-of-inertia.php#:~:text=Moment%20of%20Inertia%20of%20Common%20Shapes

    void *unique_data;                      // Unique data about the collider (radius, width, height, etc.)

    // When collisions are detected, the total amount the physics node will
    // need to move to be away from all the collided nodes is stored here
    float total_position_correction_x;
    float total_position_correction_y;

    mp_obj_t physics_tick_cb;
    mp_obj_t tick_cb;
    mp_obj_t on_collide_cb;
    mp_obj_t on_separate_cb;
    linked_list_node *physics_list_node;    // All physics 2d nodes get added to a list that is easy to traverse
}engine_physics_node_base_t;

void physics_node_base_apply_impulse_base(engine_physics_node_base_t *physics_node_base, float impulse_x, float impulse_y, float position_x, float position_y);

// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool physics_node_base_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination);

// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool physics_node_base_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination);


#endif  // PHYSICS_NODE_BASE_H