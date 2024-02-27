# Create an INTERFACE library for our CPP module..
add_library(usermod_engine INTERFACE)

set(ENGINE_MOD_DIR ${CMAKE_CURRENT_LIST_DIR})

# Add our source files to the lib
target_sources(usermod_engine INTERFACE
    ${ENGINE_MOD_DIR}/engine.c
    ${ENGINE_MOD_DIR}/math/engine_math.c
    ${ENGINE_MOD_DIR}/draw/engine_draw_module.c
    ${ENGINE_MOD_DIR}/math/engine_math_module.c
    ${ENGINE_MOD_DIR}/nodes/engine_nodes_module.c
    ${ENGINE_MOD_DIR}/input/engine_input_module.c
    ${ENGINE_MOD_DIR}/input/engine_input_rp3.c
    ${ENGINE_MOD_DIR}/input/engine_input_common.c
    ${ENGINE_MOD_DIR}/nodes/node_base.c
    ${ENGINE_MOD_DIR}/nodes/empty_node.c
    ${ENGINE_MOD_DIR}/nodes/3d/camera_node.c
    ${ENGINE_MOD_DIR}/nodes/3d/voxelspace_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/sprite_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/rectangle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/line_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/polygon_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/circle_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/physics_2d_node.c
    ${ENGINE_MOD_DIR}/nodes/2d/text_2d_node.c
    ${ENGINE_MOD_DIR}/math/vector3.c
    ${ENGINE_MOD_DIR}/math/vector2.c
    ${ENGINE_MOD_DIR}/math/rectangle.c
    ${ENGINE_MOD_DIR}/debug/engine_debug_module.c
    ${ENGINE_MOD_DIR}/debug/debug_print.c
    ${ENGINE_MOD_DIR}/utility/linked_list.c
    ${ENGINE_MOD_DIR}/utility/engine_time.c
    ${ENGINE_MOD_DIR}/utility/engine_file.c
    ${ENGINE_MOD_DIR}/utility/engine_utilities_module.c
    ${ENGINE_MOD_DIR}/utility/engine_mp.c
    ${ENGINE_MOD_DIR}/engine_object_layers.c
    ${ENGINE_MOD_DIR}/engine_cameras.c
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
    ${ENGINE_MOD_DIR}/resources/engine_font_resource.c
    ${ENGINE_MOD_DIR}/resources/engine_wave_sound_resource.c
    ${ENGINE_MOD_DIR}/physics/engine_physics_module.c
    ${ENGINE_MOD_DIR}/physics/engine_physics.c
    ${ENGINE_MOD_DIR}/physics/collision_shapes/polygon_collision_shape_2d.c
    ${ENGINE_MOD_DIR}/physics/collision_contact_2d.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_engine INTERFACE
    ${ENGINE_MOD_DIR}
)

# target_link_libraries(usermod_engine INTERFACE -llfs2)


# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_engine)
