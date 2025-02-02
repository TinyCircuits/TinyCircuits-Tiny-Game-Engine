# Create an INTERFACE library for our CPP module..
add_library(usermod_engine INTERFACE)

set(ENGINE_MOD_DIR ${CMAKE_CURRENT_LIST_DIR})

pico_generate_pio_header(usermod_engine ${ENGINE_MOD_DIR}/display/pwm.pio)

# Add our source files to the lib
target_sources(usermod_engine INTERFACE
    ${ENGINE_MOD_DIR}/engine_main.c
    ${ENGINE_MOD_DIR}/engine.c
    ${ENGINE_MOD_DIR}/engine_collections.c
    ${ENGINE_MOD_DIR}/fault/engine_fault.c
    ${ENGINE_MOD_DIR}/fault/engine_fault_rp3.c
    ${ENGINE_MOD_DIR}/fault/engine_fault_report.c
    ${ENGINE_MOD_DIR}/math/engine_math.c
    ${ENGINE_MOD_DIR}/draw/engine_draw_module.c
    ${ENGINE_MOD_DIR}/draw/engine_color.c
    ${ENGINE_MOD_DIR}/draw/engine_shader.c
    ${ENGINE_MOD_DIR}/math/engine_math_module.c
    ${ENGINE_MOD_DIR}/nodes/engine_nodes_module.c
    ${ENGINE_MOD_DIR}/io/engine_io_module.c
    ${ENGINE_MOD_DIR}/io/engine_io_buttons.c
    ${ENGINE_MOD_DIR}/io/engine_io_rp3.c
    ${ENGINE_MOD_DIR}/nodes/node_base.c
    ${ENGINE_MOD_DIR}/nodes/physics_node_base.c
    ${ENGINE_MOD_DIR}/nodes/empty_node.c
    ${ENGINE_MOD_DIR}/nodes/3D/camera_node.c
    ${ENGINE_MOD_DIR}/nodes/3D/voxelspace_node.c
    ${ENGINE_MOD_DIR}/nodes/3D/voxelspace_sprite_node.c
    ${ENGINE_MOD_DIR}/nodes/3D/mesh_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/sprite_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/rectangle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/line_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/circle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/physics_rectangle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/physics_circle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/text_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/gui_button_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2D/gui_bitmap_button_2d_node.c
    ${ENGINE_MOD_DIR}/math/vector3.c
    ${ENGINE_MOD_DIR}/math/matrix4x4.c
    ${ENGINE_MOD_DIR}/math/vector2.c
    ${ENGINE_MOD_DIR}/math/rectangle.c
    ${ENGINE_MOD_DIR}/debug/engine_debug_module.c
    ${ENGINE_MOD_DIR}/debug/debug_print.c
    ${ENGINE_MOD_DIR}/utility/linked_list.c
    ${ENGINE_MOD_DIR}/utility/engine_time.c
    ${ENGINE_MOD_DIR}/utility/engine_file.c
    ${ENGINE_MOD_DIR}/utility/engine_mp.c
    ${ENGINE_MOD_DIR}/utility/engine_bit_collection.c
    ${ENGINE_MOD_DIR}/engine_object_layers.c
    ${ENGINE_MOD_DIR}/engine_gui.c
    ${ENGINE_MOD_DIR}/display/engine_display.c
    # ${ENGINE_MOD_DIR}/display/engine_display_driver_rp2_st7789.c
    ${ENGINE_MOD_DIR}/display/engine_display_driver_rp2_gc9107.c
    ${ENGINE_MOD_DIR}/display/engine_display_common.c
    ${ENGINE_MOD_DIR}/draw/engine_display_draw.c
    ${ENGINE_MOD_DIR}/audio/engine_audio_module.c
    ${ENGINE_MOD_DIR}/audio/engine_audio_channel.c
    ${ENGINE_MOD_DIR}/resources/engine_resource_module.c
    ${ENGINE_MOD_DIR}/resources/engine_resource_manager.c
    ${ENGINE_MOD_DIR}/resources/engine_texture_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_mesh_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_font_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_wave_sound_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_tone_sound_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_rtttl_sound_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_noise_resource.c
    ${ENGINE_MOD_DIR}/physics/engine_physics_module.c
    ${ENGINE_MOD_DIR}/physics/engine_physics.c
    ${ENGINE_MOD_DIR}/physics/engine_physics_ids.c
    ${ENGINE_MOD_DIR}/physics/engine_physics_collision.c
    ${ENGINE_MOD_DIR}/physics/collision_contact_2d.c
    ${ENGINE_MOD_DIR}/animation/engine_animation_module.c
    ${ENGINE_MOD_DIR}/animation/engine_animation_tween.c
    ${ENGINE_MOD_DIR}/animation/engine_animation_delay.c
    ${ENGINE_MOD_DIR}/save/engine_save_module.c
    ${ENGINE_MOD_DIR}/save/engine_save.c
    ${ENGINE_MOD_DIR}/time/engine_rtc.c
    ${ENGINE_MOD_DIR}/time/engine_time_module.c
    ${ENGINE_MOD_DIR}/link/engine_link_module.c
    ${ENGINE_MOD_DIR}/link/engine_link_rp3.c

    ${ENGINE_MOD_DIR}/../lib/bm8563/bm8563.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_engine INTERFACE
    ${ENGINE_MOD_DIR}
    ${ENGINE_MOD_DIR}/../../lib/tinyusb/src
)

# target_link_libraries(usermod_engine INTERFACE -llfs2)


# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_engine)
