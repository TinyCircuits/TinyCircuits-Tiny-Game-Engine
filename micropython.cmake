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
    ${CMAKE_CURRENT_LIST_DIR}/physics/engine_physics.c

    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/chipmunk.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpArbiter.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpArray.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpBBTree.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpBody.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpCollision.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpConstraint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpDampedRotarySpring.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpDampedSpring.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpGearJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpGrooveJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpHashSet.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpHastySpace.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpMarch.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpPinJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpPivotJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpPolyline.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpPolyShape.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpRatchetJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpRobust.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpRotaryLimitJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpShape.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSimpleMotor.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSlideJoint.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpace.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpaceComponent.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpaceDebug.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpaceHash.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpaceQuery.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpaceStep.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSpatialIndex.c
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/src/cpSweep1D.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_engine INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/libs/Chipmunk2D/include
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_engine)