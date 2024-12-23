ENGINE_MOD_DIR := $(USERMOD_DIR)

# Add our source files to the respective variables.
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_main.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_collections.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/fault/engine_fault.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/fault/engine_fault_unix.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/fault/engine_fault_report.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/engine_math.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_draw_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_color.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_shader.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/engine_math_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/engine_nodes_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/io/engine_io_buttons.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/io/engine_io_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/io/engine_io_sdl.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/node_base.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/physics_node_base.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/empty_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/3D/camera_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/3D/voxelspace_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/3D/voxelspace_sprite_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/3D/mesh_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/sprite_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/rectangle_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/line_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/circle_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/physics_rectangle_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/physics_circle_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/text_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/gui_button_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/nodes/2D/gui_bitmap_button_2d_node.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/vector3.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/matrix4x4.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/vector2.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/math/rectangle.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/debug/engine_debug_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/debug/debug_print.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/linked_list.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/engine_time.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/engine_file.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/engine_mp.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/utility/engine_bit_collection.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_object_layers.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/engine_gui.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display_driver_unix_sdl.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/display/engine_display_common.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/draw/engine_display_draw.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/audio/engine_audio_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/audio/engine_audio_channel.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_resource_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_resource_manager.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_texture_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_mesh_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_font_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_wave_sound_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_tone_sound_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_rtttl_sound_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/resources/engine_noise_resource.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics_ids.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/engine_physics_collision.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/physics/collision_contact_2d.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/animation/engine_animation_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/animation/engine_animation_tween.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/animation/engine_animation_delay.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/save/engine_save_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/save/engine_save.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/time/engine_rtc.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/time/engine_time_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/link/engine_link_module.c
SRC_USERMOD += $(ENGINE_MOD_DIR)/link/engine_link_unix.c

SRC_USERMOD += $(ENGINE_MOD_DIR)/../lib/bm8563/bm8563.c


# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)

CXXFLAGS_USERMOD += -I$(ENGINE_MOD_DIR)
CXXFLAGS_USERMOD += -std=c++11

# We use C++ features so have to link against the standard library.
LDFLAGS_USERMOD += -lstdc++

LDFLAGS_EXTRA += -lSDL2 -lpthread
