# Create an INTERFACE library for our CPP module..
add_library(usermod_engine INTERFACE)

# Add our source files to the lib
target_sources(usermod_engine INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/engine.c
    ${CMAKE_CURRENT_LIST_DIR}/draw/engine_draw_module.c
    ${CMAKE_CURRENT_LIST_DIR}/math/engine_math_module.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/engine_nodes_module.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_module.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_rp2.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_common.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/empty_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/3d/camera_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/sprite_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/rectangle_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/physics_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/math/vector3.c
    ${CMAKE_CURRENT_LIST_DIR}/math/vector2.c
    ${CMAKE_CURRENT_LIST_DIR}/math/rectangle.c
    ${CMAKE_CURRENT_LIST_DIR}/debug/engine_debug_module.c
    ${CMAKE_CURRENT_LIST_DIR}/debug/debug_print.c
    ${CMAKE_CURRENT_LIST_DIR}/utility/linked_list.c
    ${CMAKE_CURRENT_LIST_DIR}/utility/engine_time.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/mpfile/mpfile.c
    ${CMAKE_CURRENT_LIST_DIR}/engine_object_layers.c
    ${CMAKE_CURRENT_LIST_DIR}/engine_cameras.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display_driver_rp2_st7789.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display_common.c
    ${CMAKE_CURRENT_LIST_DIR}/draw/engine_display_draw.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_resources_module.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_texture_resource.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_sound_resource.c
    ${CMAKE_CURRENT_LIST_DIR}/physics/engine_physics_module.c

    ${CMAKE_CURRENT_LIST_DIR}/physics/engine_physics.cpp

    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_broad_phase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_chain_shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_circle_shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_collide_circle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_collide_edge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_collide_polygon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_collision.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_distance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_dynamic_tree.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_edge_shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_polygon_shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/collision/b2_time_of_impact.cpp

    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_block_allocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_math.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_stack_allocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/common/b2_timer.cpp

    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_body.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_chain_circle_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_chain_polygon_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_circle_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_contact_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_contact_solver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_distance_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_edge_circle_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_edge_polygon_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_fixture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_friction_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_gear_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_island.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_motor_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_mouse_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_polygon_circle_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_polygon_contact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_prismatic_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_pulley_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_revolute_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_weld_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_wheel_joint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_world_callbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/src/dynamics/b2_world.cpp
)

# Add the current directory as an include directory.
target_include_directories(usermod_engine INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/libs/box2d/include
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_engine)