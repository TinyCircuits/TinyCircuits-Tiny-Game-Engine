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
        float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

        // Get polygon scale and incorporate camera zoom
        bool polygon_outlined = mp_obj_get_float(mp_load_attr(polygon_node_base->attr_accessor, MP_QSTR_outline));
        float polygon_scale = mp_obj_get_float(mp_load_attr(polygon_node_base->attr_accessor, MP_QSTR_scale));
        polygon_scale = polygon_scale * camera_zoom;

        uint16_t polygon_color = mp_obj_get_int(mp_load_attr(polygon_node_base->attr_accessor, MP_QSTR_color));

        float camera_resolved_hierarchy_x = 0.0f;
        float camera_resolved_hierarchy_y = 0.0f;
        float camera_resolved_hierarchy_rotation = 0.0f;
        node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);
        camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

        // Get the absolute position of the node depending in parents
        float polygon_resolved_hierarchy_x = 0.0f;
        float polygon_resolved_hierarchy_y = 0.0f;
        float polygon_resolved_hierarchy_rotation = 0.0f;
        node_base_get_child_absolute_xy(&polygon_resolved_hierarchy_x, &polygon_resolved_hierarchy_y, &polygon_resolved_hierarchy_rotation, self_in);

        float polygon_rotated_x = polygon_resolved_hierarchy_x-camera_resolved_hierarchy_x;
        float polygon_rotated_y = polygon_resolved_hierarchy_y-camera_resolved_hierarchy_y;

        // Scale transformation due to camera zoom
        engine_math_scale_point(&polygon_rotated_x, &polygon_rotated_y, camera_position->x, camera_position->y, camera_zoom);

        // Rotate polygon origin about the camera
        engine_math_rotate_point(&polygon_rotated_x, &polygon_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

        polygon_rotated_x += camera_viewport->width/2;
        polygon_rotated_y += camera_viewport->height/2;

        // Time to draw the polygon!
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

        // With the 'center' calculated, rotate each vertex before
        // drawing the line between each pair of points. After rotating
        // offset by node postion amount
        float last_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[0])->x;
        float last_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[0])->y;

        // Scale the vertices away from the center
        engine_math_scale_point(&last_rotated_vertex_x, &last_rotated_vertex_y, center_x, center_y, polygon_scale);

        engine_math_rotate_point(&last_rotated_vertex_x, &last_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation);
        last_rotated_vertex_x += polygon_rotated_x;
        last_rotated_vertex_y += polygon_rotated_y;

        // Save this for connecting the end of the polygon to the start
        float first_rotated_vertex_x = last_rotated_vertex_x;
        float first_rotated_vertex_y = last_rotated_vertex_y;
        
        float current_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[1])->x;
        float current_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[1])->y;

        // Scale the vertices away from the center
        engine_math_scale_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_scale);

        engine_math_rotate_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation);
        current_rotated_vertex_x += polygon_rotated_x;
        current_rotated_vertex_y += polygon_rotated_y;

        if(polygon_outlined == false){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Polygon2DNode: ERROR: Filled rendering is not implemented yet, please set 'outline' to True"));
        }else{
            for(uint16_t ivx=2; ivx<polygon_vertex_list->len; ivx++){
                engine_draw_line(polygon_color, last_rotated_vertex_x, last_rotated_vertex_y, current_rotated_vertex_x, current_rotated_vertex_y, camera_node);

                // To avoid doing more rotations than needed, save the current to the last
                last_rotated_vertex_x = current_rotated_vertex_x;
                last_rotated_vertex_y = current_rotated_vertex_y;

                current_rotated_vertex_x = ((vector2_class_obj_t*)polygon_vertex_list->items[ivx])->x;
                current_rotated_vertex_y = ((vector2_class_obj_t*)polygon_vertex_list->items[ivx])->y;
                // Scale the vertices away from the center
                engine_math_scale_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_scale);

                engine_math_rotate_point(&current_rotated_vertex_x, &current_rotated_vertex_y, center_x, center_y, polygon_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation);

                // Incorporate the position of the node
                current_rotated_vertex_x += polygon_rotated_x;
                current_rotated_vertex_y += polygon_rotated_y;
            }

            engine_draw_line(polygon_color, last_rotated_vertex_x, last_rotated_vertex_y, current_rotated_vertex_x, current_rotated_vertex_y, camera_node);
            engine_draw_line(polygon_color, current_rotated_vertex_x, current_rotated_vertex_y, first_rotated_vertex_x, first_rotated_vertex_y, camera_node);
        }
    }
                                               
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(polygon_2d_node_class_draw_obj, polygon_2d_node_class_draw);


/*  --- doc ---
    NAME: Polygon2DNode
    DESC: Node for drawing, currently, outlined polygons (TODO: filled) given a list of vertices
    PARAM:  [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    PARAM:  [type=list]                       [name=vertices]                   [value=list of {ref_link:Vector2}s]
    PARAM:  [type=int]                        [name=color]                      [value={any 16-bit RGB565 integer]
    PARAM:  [type=boolean]                    [name=outline]                    [value=True or False]
    PARAM:  [type={ref_link:Vector3}]         [name=rotation]                   [value={ref_link:Vector3}]
    PARAM:  [type=float]                      [name=scale]                      [value=any (TODO: make this a Vector2?)]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    ATTR:   [type=list]                       [name=vertices]                   [value=list of {ref_link:Vector2}s]
    ATTR:   [type=int]                        [name=color]                      [value={any 16-bit RGB565 integer]
    ATTR:   [type=boolean]                    [name=outline]                    [value=True or False]
    ATTR:   [type={ref_link:Vector3}]         [name=rotation]                   [value={ref_link:Vector3}]
    ATTR:   [type=float]                      [name=scale]                      [value=any (TODO: make this a Vector2?)]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    OVRR:   [type=function]                   [name={ref_link:draw}]            [value=function]
*/
mp_obj_t polygon_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Polygon2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_vertices,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, vertices, color, outline, rotation, scale};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector2_class_type){
        // Using positional arguments but the type of the first one isn't
        // as expected. Must be the child class
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);
        inherited = true;
    }else{
        // Whether we're using positional arguments or not, prase them this
        // way. It's a requirement that the child class be passed using position.
        // Adjust what and where the arguments are parsed, since not inherited based
        // on the first argument
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args)-1, allowed_args+1, parsed_args+1);
        inherited = false;
    }

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[vertices].u_obj == MP_OBJ_NULL) parsed_args[vertices].u_obj = mp_obj_new_list(0, NULL);
    if(parsed_args[color].u_obj == MP_OBJ_NULL) parsed_args[color].u_obj = mp_obj_new_int(0xffff);
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_bool(false);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = mp_obj_new_float(1.0f);

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

    if(inherited == false){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_polygon_2d_node_class_obj_t *polygon_2d_node = m_malloc(sizeof(engine_polygon_2d_node_class_obj_t));
        node_base->node = polygon_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&polygon_2d_node_class_draw_obj);

        polygon_2d_node->position = parsed_args[position].u_obj;
        polygon_2d_node->vertices = parsed_args[vertices].u_obj;
        polygon_2d_node->color = parsed_args[color].u_obj;
        polygon_2d_node->outline = parsed_args[outline].u_obj;   // TODO: this should be false by default but don't have a filled renderer yet...
        polygon_2d_node->rotation = parsed_args[rotation].u_obj;
        polygon_2d_node->scale = parsed_args[scale].u_obj;
    }else if(inherited = true){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = parsed_args[child_class].u_obj;
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

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_vertices, parsed_args[vertices].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_color, parsed_args[color].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_outline, parsed_args[outline].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_scale, parsed_args[scale].u_obj);
        
    }

    return MP_OBJ_FROM_PTR(node_base);
}


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
            case MP_QSTR_get_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
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
            case MP_QSTR_vertices:
                destination[0] = self->vertices;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_outline:
                destination[0] = self->outline;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_vertices:
                self->vertices = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            case MP_QSTR_outline:
                self->outline = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
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