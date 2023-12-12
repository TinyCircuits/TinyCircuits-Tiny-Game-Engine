# Create an INTERFACE library for our CPP module..
add_library(usermod_engine INTERFACE)

# Add our source files to the lib
target_sources(usermod_engine INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/engine.c
    ${CMAKE_CURRENT_LIST_DIR}/math/engine_math.c
    ${CMAKE_CURRENT_LIST_DIR}/draw/engine_draw_module.c
    ${CMAKE_CURRENT_LIST_DIR}/math/engine_math_module.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/engine_nodes_module.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_module.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_rp2.c
    ${CMAKE_CURRENT_LIST_DIR}/input/engine_input_common.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_base.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/empty_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/3d/camera_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/sprite_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/rectangle_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/circle_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/nodes/2d/physics_2d_node.c
    ${CMAKE_CURRENT_LIST_DIR}/math/vector3.c
    ${CMAKE_CURRENT_LIST_DIR}/math/vector2.c
    ${CMAKE_CURRENT_LIST_DIR}/math/rectangle.c
    ${CMAKE_CURRENT_LIST_DIR}/debug/engine_debug_module.c
    ${CMAKE_CURRENT_LIST_DIR}/debug/debug_print.c
    ${CMAKE_CURRENT_LIST_DIR}/utility/linked_list.c
    ${CMAKE_CURRENT_LIST_DIR}/utility/engine_time.c
    ${CMAKE_CURRENT_LIST_DIR}/utility/engine_file.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/mpfile/mpfile.c
    ${CMAKE_CURRENT_LIST_DIR}/engine_object_layers.c
    ${CMAKE_CURRENT_LIST_DIR}/engine_cameras.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display_driver_rp2_st7789.c
    ${CMAKE_CURRENT_LIST_DIR}/display/engine_display_common.c
    ${CMAKE_CURRENT_LIST_DIR}/draw/engine_display_draw.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_resource_module.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_resource_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_texture_resource.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_font_resource.c
    ${CMAKE_CURRENT_LIST_DIR}/resources/engine_sound_resource.c
    ${CMAKE_CURRENT_LIST_DIR}/physics/engine_physics_module.c
    ${CMAKE_CURRENT_LIST_DIR}/physics/engine_physics.c
    ${CMAKE_CURRENT_LIST_DIR}/physics/shapes/engine_physics_shape_rectangle.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_engine INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# target_link_libraries(usermod_engine INTERFACE -llfs2)


# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_engine)