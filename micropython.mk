ENGINE_MOD_DIR := $(USERMOD_DIR)

# Add our source files to the respective variables.
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_draw_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/engine_math_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/engine_nodes_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/input/engine_input_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/input/engine_input_sdl.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/input/engine_input_common.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/empty_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/3d/camera_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2d/sprite_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2d/rectangle_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2d/physics_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/vector3.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/vector2.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/rectangle.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/debug/engine_debug_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/debug/debug_print.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/linked_list.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/engine_time.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/libs/mpfile/mpfile.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_object_layers.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_cameras.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display_driver_unix_sdl.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display_common.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_display_draw.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_resources_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_texture_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_sound_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics_module.c

SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/physics/engine_physics.cpp

SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_broad_phase.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_chain_shape.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_circle_shape.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_collide_circle.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_collide_edge.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_collide_polygon.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_collision.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_distance.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_dynamic_tree.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_edge_shape.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_polygon_shape.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/collision/b2_time_of_impact.cpp

SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_block_allocator.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_draw.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_math.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_settings.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_stack_allocator.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/common/b2_timer.cpp

SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_body.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_chain_circle_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_chain_polygon_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_circle_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_contact_manager.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_contact_solver.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_distance_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_edge_circle_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_edge_polygon_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_fixture.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_friction_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_gear_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_island.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_motor_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_mouse_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_polygon_circle_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_polygon_contact.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_prismatic_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_pulley_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_revolute_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_weld_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_wheel_joint.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_world_callbacks.cpp
SRC_USERMOD_CXX  += $(ENGINE_MOD_DIR)/libs/box2d/src/dynamics/b2_world.cpp

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)
CFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)/libs/box2d/include/

CXXFLAGS_USERMOD += -std=c++11
CXXFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)
CXXFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)/libs/box2d/include/

LDFLAGS_EXTRA += -lSDL2