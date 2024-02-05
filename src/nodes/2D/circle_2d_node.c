#include "circle_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"

// Class required functions
STATIC void circle_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Circle2DNode");
}


STATIC mp_obj_t circle_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Circle2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(circle_2d_node_class_tick_obj, circle_2d_node_class_tick);


STATIC mp_obj_t circle_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Circle2DNode: Drawing");
    
    engine_node_base_t *circle_node_base = self_in;
    engine_node_base_t *camera_node_base = camera_node;

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

    float circle_scale =  mp_obj_get_float(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_scale));
    float circle_radius =  (mp_obj_get_float(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_radius)));
    bool circle_outlined = mp_obj_get_int(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_outline));

    // The final circle radius to draw the circle at is a combination of
    // the set radius, times the set scale, times the set camera zoom
    circle_radius = (circle_radius*circle_scale*camera_zoom);
    
    float circle_radius_sqr = circle_radius * circle_radius;
    mp_int_t circle_color = mp_obj_get_int(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_color));

    float camera_resolved_hierarchy_x = 0.0f;
    float camera_resolved_hierarchy_y = 0.0f;
    float camera_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);
    camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

    float circle_resolved_hierarchy_x = 0.0f;
    float circle_resolved_hierarchy_y = 0.0f;
    float circle_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&circle_resolved_hierarchy_x, &circle_resolved_hierarchy_y, &circle_resolved_hierarchy_rotation, self_in);

    // Store the non-rotated x and y for a second
    float circle_rotated_x = circle_resolved_hierarchy_x-camera_resolved_hierarchy_x;
    float circle_rotated_y = circle_resolved_hierarchy_y-camera_resolved_hierarchy_y;

    // Scale transformation due to camera zoom
    engine_math_scale_point(&circle_rotated_x, &circle_rotated_y, camera_position->x, camera_position->y, camera_zoom);

    // Rotate circle origin about the camera
    engine_math_rotate_point(&circle_rotated_x, &circle_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

    circle_rotated_x += camera_viewport->width/2;
    circle_rotated_y += camera_viewport->height/2;

    if(circle_outlined == false){
        // https://stackoverflow.com/a/59211338
        for(int x=-circle_radius; x<(int)circle_radius; x++){
            int hh = (int)sqrt(circle_radius_sqr - x * x);
            int rx = (int)circle_rotated_x + x;
            int ph = (int)circle_rotated_y + hh;

            for(int y=(int)circle_rotated_y-hh; y<ph; y++){
                engine_draw_pixel(circle_color, rx, y);
            }
        }
    }else{
        // https://stackoverflow.com/a/58629898
        float angle_increment = acosf(1 - 1/circle_radius) * 2.0f;   // Multiply by 2.0 since care about speed and not accuracy as much

        for(float angle = 0; angle <= 90; angle += angle_increment){
                float cx = circle_radius * cosf(angle);
                float cy = circle_radius * sinf(angle);
                
                // Bottom right quadrant of the circle
                int brx = circle_rotated_x+cx;
                int bry = circle_rotated_y+cy;

                // Bottom left quadrant of the circle
                int blx = circle_rotated_x-cx;
                int bly = circle_rotated_y+cy;

                // Top right quadrant of the circle
                int trx = circle_rotated_x+cx;
                int try = circle_rotated_y-cy;

                // Top left quadrant of the circle
                int tlx = circle_rotated_x-cx;
                int tly = circle_rotated_y-cy;

                engine_draw_pixel(circle_color, brx, bry);
                engine_draw_pixel(circle_color, blx, bly);
                engine_draw_pixel(circle_color, trx, try);
                engine_draw_pixel(circle_color, tlx, tly);
        }
    }
                                               
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(circle_2d_node_class_draw_obj, circle_2d_node_class_draw);


mp_obj_t circle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Circle2DNode");

    engine_circle_2d_node_common_data_t *common_data = malloc(sizeof(engine_circle_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_circle_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_CIRCLE_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_circle_2d_node_class_obj_t *circle_2d_node = m_malloc(sizeof(engine_circle_2d_node_class_obj_t));
        node_base->node = circle_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_draw_obj);

        circle_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        circle_2d_node->radius = mp_obj_new_float(5.0f);
        circle_2d_node->rotation = mp_obj_new_float(0.0f);
        circle_2d_node->color = mp_obj_new_int(0xffff);
        circle_2d_node->scale = mp_obj_new_float(1.0f);
        circle_2d_node->outline = mp_obj_new_bool(false);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_radius, mp_obj_new_float(5.0f));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_color, mp_obj_new_int(0xffff));
        mp_store_attr(node_base->node, MP_QSTR_scale, mp_obj_new_float(1.0f));
        mp_store_attr(node_base->node, MP_QSTR_outline, mp_obj_new_bool(false));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Too many arguments passed to Circle2DNode constructor!"));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


/* --- doc ---
   NAME: Circle2DNode
   DESC: Simple node that draws a colored circle given a radius
   ATTR: [type=function]            [name={ref_link:add_child}]        [value=function]                                        
   ATTR: [type=function]            [name={ref_link:remove_child}]     [value=function]                                        
   ATTR: [type=function]            [name={ref_link:set_layer}]        [value=function]                                        
   ATTR: [type=function]            [name={ref_link:get_layer}]        [value=function]                                        
   ATTR: [type={ref_link:Vector2}]  [name=position]                    [value={ref_link:Vector2}]                                
   ATTR: [type=float]               [name=radius]                      [value=any]                                             
   ATTR: [type=float]               [name=rotation]                    [value=any]                                             
   ATTR: [type=int]                 [name=color]                       [value=0 ~ 65535 (16-bit RGB565 0bRRRRRGGGGGGBBBBB)]    
   ATTR: [type=float]               [name=scale]                       [value=any]           
   ATTR: [type=bool]                [name=outline]                     [value=True or False]     
   OVRR: [type=function]            [name={ref_link:tick}]             [value=function]
   OVRR: [type=function]            [name={ref_link:draw}]             [value=function]                              
*/ 
STATIC void circle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Circle2DNode attr");

    engine_circle_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_radius:
                destination[0] = self->radius;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
            break;
            case MP_QSTR_outline:
                destination[0] = self->outline;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_radius:
                self->radius = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
            break;
            case MP_QSTR_outline:
                self->outline = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t circle_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(circle_2d_node_class_locals_dict, circle_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_circle_2d_node_class_type,
    MP_QSTR_Circle2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, circle_2d_node_class_new,
    print, circle_2d_node_class_print,
    attr, circle_2d_node_class_attr,
    locals_dict, &circle_2d_node_class_locals_dict
);
