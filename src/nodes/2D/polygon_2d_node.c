#include "polygon_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/rectangle.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "draw/engine_display_draw.h"
#include "resources/engine_texture_resource.h"
#include "utility/engine_file.h"
#include "math/engine_math.h"
#include "utility/engine_time.h"


// Class required functions
STATIC void polygon_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_FORCE_PRINTF("print(): Polygon2DNode");
}


STATIC mp_obj_t polygon_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Polygon2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(polygon_2d_node_class_tick_obj, polygon_2d_node_class_tick);


STATIC mp_obj_t polygon_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Polygon2DNode: Drawing");

    engine_node_base_t *polygon_node_base = self_in;
    mp_obj_list_t *polygon_vertex_list = mp_load_attr(polygon_node_base->attr_accessor, MP_QSTR_vertices);

    // Only draw outline polygon if there are enough vertices
    if(polygon_vertex_list->len >= 2){
        engine_node_base_t *camera_node_base = camera_node;
        vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
        vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
        rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
        uint16_t polygon_color = mp_obj_get_int(mp_load_attr(polygon_node_base->attr_accessor, MP_QSTR_color));


        // Get the absolute position of the node depending in parents
        float camera_resolved_hierarchy_x = 0.0f;
        float camera_resolved_hierarchy_y = 0.0f;
        float camera_resolved_hierarchy_rotation = 0.0f;

        node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);

        float camera_rotated_x = camera_resolved_hierarchy_x;
        float camera_rotated_y = camera_resolved_hierarchy_y;

        // Rotate camera origin about the camera
        engine_math_rotate_point(&camera_rotated_x, &camera_rotated_y, (float)camera_viewport->width/2, (float)camera_viewport->height/2, (float)camera_rotation->z);


        // Calculate the average postion of all vertices and
        // rotate them about that. Afterwards offset by node
        // position (to the user, if they create a box they
        // should expect that the node postion is the center
        // of that box)
        float center_x = 0.0f;
        float center_y = 0.0f;

        for(uint16_t ivx=0; ivx<polygon_vertex_list->len; ivx++){
            vector2_class_obj_t *vertex = polygon_vertex_list->items[ivx];
            center_x += vertex->x;
            center_y += vertex->y;
        }

        center_x = center_x / polygon_vertex_list->len;
        center_y = center_y / polygon_vertex_list->len;

        // Get the absolute position of the node depending in parents
        float polygon_resolved_hierarchy_x = 0.0f;
        float polygon_resolved_hierarchy_y = 0.0f;
        float polygon_resolved_hierarchy_rotation = 0.0f;

        node_base_get_child_absolute_xy(&polygon_resolved_hierarchy_x, &polygon_resolved_hierarchy_y, &polygon_resolved_hierarchy_rotation, self_in);

        float polygon_rotated_x = polygon_resolved_hierarchy_x-((float)camera_rotated_x);
        float polygon_rotated_y = polygon_resolved_hierarchy_y-((float)camera_rotated_y);

        // Rotate polygon origin about the camera
        engine_math_rotate_point(&polygon_rotated_x, &polygon_rotated_y, (float)camera_viewport->width/2, (float)camera_viewport->height/2, (float)camera_rotation->z);

        // With the 'center' calculated, rotate each vertex before
        // drawing the line between each pair of points. After rotating
        // offset by node postion amount
        float last_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[0])->x;
        float last_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[0])->y;
        engine_math_rotate_point(&last_rotated_vertex_x, &last_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation);
        last_rotated_vertex_x += polygon_rotated_x;
        last_rotated_vertex_y += polygon_rotated_y;

        // Save this for connecting the end of the polygon to the start
        float first_rotated_vertex_x = last_rotated_vertex_x;
        float first_rotated_vertex_y = last_rotated_vertex_y;
        
        float current_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[1])->x;
        float current_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[1])->y;
        engine_math_rotate_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation);
        current_rotated_vertex_x += polygon_rotated_x;
        current_rotated_vertex_y += polygon_rotated_y;

        for(uint16_t ivx=2; ivx<polygon_vertex_list->len; ivx++){
            engine_draw_line(polygon_color, last_rotated_vertex_x, last_rotated_vertex_y, current_rotated_vertex_x, current_rotated_vertex_y, camera_node);

            // To avoid doing more rotations than needed, save the current to the last
            last_rotated_vertex_x = current_rotated_vertex_x;
            last_rotated_vertex_y = current_rotated_vertex_y;

            current_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[ivx])->x;
            current_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[ivx])->y;
            engine_math_rotate_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation);

            // Incorporate the postion of the node
            current_rotated_vertex_x += polygon_rotated_x;
            current_rotated_vertex_y += polygon_rotated_y;
        }

        engine_draw_line(polygon_color, last_rotated_vertex_x, last_rotated_vertex_y, current_rotated_vertex_x, current_rotated_vertex_y, camera_node);
        engine_draw_line(polygon_color, current_rotated_vertex_x, current_rotated_vertex_y, first_rotated_vertex_x, first_rotated_vertex_y, camera_node);
    }
                                               
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(polygon_2d_node_class_draw_obj, polygon_2d_node_class_draw);


mp_obj_t polygon_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Polygon2DNode");

    engine_polygon_2d_node_common_data_t *common_data = malloc(sizeof(engine_polygon_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_polygon_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_POLYGON_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_polygon_2d_node_class_obj_t *polygon_2d_node = m_malloc(sizeof(engine_polygon_2d_node_class_obj_t));
        node_base->node = polygon_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_draw_obj);

        polygon_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        polygon_2d_node->color = mp_obj_new_int(0xffff);
        polygon_2d_node->rotation = mp_obj_new_float(0.0f);
        polygon_2d_node->vertices = mp_obj_new_list(0, NULL);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_color, mp_obj_new_int(0xffff));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_vertices, mp_obj_new_list(0, NULL));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Too many arguments passed to Polygon2DNode constructor!"));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC void polygon_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Polygon2DNode attr");

    engine_polygon_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_add_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_remove_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_node_base:
                destination[0] = self_in;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_vertices:
                destination[0] = self->vertices;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_vertices:
                self->vertices = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t polygon_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(polygon_2d_node_class_locals_dict, polygon_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_polygon_2d_node_class_type,
    MP_QSTR_Polygon2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, polygon_2d_node_class_new,
    print, polygon_2d_node_class_print,
    attr, polygon_2d_node_class_attr,
    locals_dict, &polygon_2d_node_class_locals_dict
);