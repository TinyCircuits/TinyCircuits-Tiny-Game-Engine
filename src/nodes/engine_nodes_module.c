#include "py/obj.h"

#include "empty_node.h"
#include "3d/camera_node.h"
#include "3d/voxelspace_node.h"
#include "2d/sprite_2d_node.h"
#include "2d/rectangle_2d_node.h"
#include "2d/line_2d_node.h"
#include "2d/circle_2d_node.h"
#include "2d/physics_rectangle_2d_node.h"
#include "2d/text_2d_node.h"


/*  --- doc ---
    NAME: engine_nodes
    DESC: Module containing nodes that get drawn to the framebuffer. Each node also has a tick function that can be overridden to run code every game loop.
    ATTR: [type=object]   [name={ref_link:EmptyNode}]           [value=object] 
    ATTR: [type=object]   [name={ref_link:CameraNode}]          [value=object]
    ATTR: [type=object]   [name={ref_link:VoxelSpaceNode}]      [value=object]
    ATTR: [type=object]   [name={ref_link:Sprite2DNode}]        [value=object]
    ATTR: [type=object]   [name={ref_link:Rectangle2DNode}]     [value=object]
    ATTR: [type=object]   [name={ref_link:Line2DNode}]          [value=object]
    ATTR: [type=object]   [name={ref_link:Polygon2DNode}]       [value=object]
    ATTR: [type=object]   [name={ref_link:Circle2DNode}]        [value=object]
    ATTR: [type=object]   [name={ref_link:Physics2DNode}]       [value=object]
    ATTR: [type=object]   [name={ref_link:Text2DNode}]          [value=object]
*/
STATIC const mp_rom_map_elem_t engine_nodes_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_nodes) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_EmptyNode), (mp_obj_t)&engine_empty_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CameraNode), (mp_obj_t)&engine_camera_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_VoxelSpaceNode), (mp_obj_t)&engine_voxelspace_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Sprite2DNode), (mp_obj_t)&engine_sprite_2d_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Rectangle2DNode), (mp_obj_t)&engine_rectangle_2d_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Line2DNode), (mp_obj_t)&engine_line_2d_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Circle2DNode), (mp_obj_t)&engine_circle_2d_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PhysicsRectangle2DNode), (mp_obj_t)&engine_physics_rectangle_2d_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Text2DNode), (mp_obj_t)&engine_text_2d_node_class_type },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_nodes_globals, engine_nodes_globals_table);

const mp_obj_module_t engine_nodes_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_nodes_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_nodes, engine_nodes_user_cmodule);