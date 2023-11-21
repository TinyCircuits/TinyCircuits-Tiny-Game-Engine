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
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)

CXXFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)
CXXFLAGS_USERMOD += -std=c++11

# We use C++ features so have to link against the standard library.
LDFLAGS_USERMOD += -lstdc++

LDFLAGS_EXTRA += -lSDL2