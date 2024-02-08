#include "sprite_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "resources/engine_texture_resource.h"
#include "utility/engine_file.h"
#include "math/engine_math.h"
#include "utility/engine_time.h"


// Class required functions
STATIC void sprite_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Sprite2DNode");
}


STATIC mp_obj_t sprite_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Sprite2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(sprite_2d_node_class_tick_obj, sprite_2d_node_class_tick);


STATIC mp_obj_t sprite_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Sprite2DNode: Drawing");

    // Decode and store properties about the rectangle and camera nodes
    engine_node_base_t *sprite_node_base = self_in;
    engine_node_base_t *camera_node_base = camera_node;
    engine_sprite_2d_node_common_data_t *sprite_common_data = sprite_node_base->node_common_data;

    vector2_class_obj_t *sprite_scale =  mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_scale);
    texture_resource_class_obj_t *sprite_texture = mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_texture);

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

    uint16_t sprite_frame_count_x = mp_obj_get_int(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_count_x));
    uint16_t sprite_frame_count_y = mp_obj_get_int(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_count_y));
    uint16_t sprite_frame_current_x = mp_obj_get_int(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_x));
    uint16_t sprite_frame_current_y = mp_obj_get_int(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_y));

    uint16_t transparent_color = mp_obj_get_int(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_transparent_color));
    uint32_t spritesheet_width = sprite_texture->width;
    uint32_t spritesheet_height = sprite_texture->height;

    uint16_t *sprite_pixel_data = (uint16_t*)sprite_texture->data;

    uint32_t sprite_frame_width = spritesheet_width/sprite_frame_count_x;
    uint32_t sprite_frame_height = spritesheet_height/sprite_frame_count_y;
    uint32_t sprite_frame_abs_x = sprite_frame_width*sprite_frame_current_x;
    uint32_t sprite_frame_abs_y = sprite_frame_height*sprite_frame_current_y;
    uint32_t sprite_frame_fb_start_index = sprite_frame_abs_y * spritesheet_width + sprite_frame_abs_x;

    float camera_resolved_hierarchy_x = 0.0f;
    float camera_resolved_hierarchy_y = 0.0f;
    float camera_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);
    camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

    float sprite_resolved_hierarchy_x = 0.0f;
    float sprite_resolved_hierarchy_y = 0.0f;
    float sprite_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&sprite_resolved_hierarchy_x, &sprite_resolved_hierarchy_y, &sprite_resolved_hierarchy_rotation, self_in);

    // Store the non-rotated x and y for a second
    float sprite_rotated_x = sprite_resolved_hierarchy_x-camera_resolved_hierarchy_x;
    float sprite_rotated_y = sprite_resolved_hierarchy_y-camera_resolved_hierarchy_y;

    // Scale transformation due to camera zoom
    engine_math_scale_point(&sprite_rotated_x, &sprite_rotated_y, camera_position->x, camera_position->y, camera_zoom);

    // Rotate sprite origin about the camera
    engine_math_rotate_point(&sprite_rotated_x, &sprite_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

    sprite_rotated_x += camera_viewport->width/2;
    sprite_rotated_y += camera_viewport->height/2;

    engine_draw_blit(sprite_pixel_data+sprite_frame_fb_start_index,
                     sprite_rotated_x, sprite_rotated_y,
                     sprite_frame_width, sprite_frame_height,
                     spritesheet_width,
                     -(sprite_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation));

    // engine_draw_blit_scale_rotate( sprite_pixel_data+sprite_frame_fb_start_index,
    //                               (int32_t)sprite_rotated_x,
    //                               (int32_t)sprite_rotated_y,
    //                               spritesheet_width,
    //                               sprite_frame_width,
    //                               sprite_frame_height,
    //                               (int32_t)((sprite_scale->x*camera_zoom)*65536 + 0.5),
    //                               (int32_t)((sprite_scale->y*camera_zoom)*65536 + 0.5),
    //                               (int16_t)(((sprite_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation))*1024 / (float)(2*PI)),
    //                               transparent_color);

    // After drawing, go to the next frame if it is time to
    float sprite_fps = mp_obj_get_float(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_fps));
    uint16_t sprite_period = (uint16_t)((1.0f/sprite_fps) * 1000.0f);

    uint32_t current_ms_time = millis();
    if(current_ms_time - sprite_common_data->time_at_last_animation_update_ms >= sprite_period){
        sprite_frame_current_x++;

        // If reach end of x-axis frames, go to the next line and restart x
        if(sprite_frame_current_x >= sprite_frame_count_x){
            sprite_frame_current_x = 0;
            sprite_frame_current_y++;
        }

        // If reach end of y-axis frames, restart at x=0 and y=0
        if(sprite_frame_current_y >= sprite_frame_count_y){
            sprite_frame_current_y = 0;
        }

        // Update/store the current frame index
        mp_store_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_x, mp_obj_new_int(sprite_frame_current_x));
        mp_store_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_y, mp_obj_new_int(sprite_frame_current_y));
        sprite_common_data->time_at_last_animation_update_ms = millis();
    }
                                               
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(sprite_2d_node_class_draw_obj, sprite_2d_node_class_draw);


/*  --- doc ---
    NAME: Sprite2DNode
    DESC: Simple 2D sprite node that can be animated or static
    PARAM:  [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    PARAM:  [type=int]                        [name=transparent_color]          [value=any 16-bit RGB565 color]
    PARAM:  [type=float]                      [name=fps]                        [value=any]
    PARAM:  [type=int]                        [name=frame_count_x]              [value=any positive integer]
    PARAM:  [type=int]                        [name=frame_count_y]              [value=any positive integer]
    PARAM:  [type=float]                      [name=rotation]                   [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    ATTR:   [type=int]                        [name=transparent_color]          [value=any 16-bit RGB565 color]
    ATTR:   [type=float]                      [name=fps]                        [value=any]
    ATTR:   [type=int]                        [name=frame_count_x]              [value=any positive integer]
    ATTR:   [type=int]                        [name=frame_count_y]              [value=any positive integer]
    ATTR:   [type=float]                      [name=rotation]                   [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=int]                        [name=frame_current_x]            [value=any positive integer]
    ATTR:   [type=int]                        [name=frame_current_y]            [value=any positive integer]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    OVRR:   [type=function]                   [name={ref_link:draw}]            [value=function]
*/
mp_obj_t sprite_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Sprite2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_texture,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_transparent_color,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_fps,                  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_frame_count_x,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_frame_count_y,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,                MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, transparent_color, fps, frame_count_x, frame_count_y, rotation, scale};
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
    if(parsed_args[texture].u_obj == MP_OBJ_NULL) parsed_args[texture].u_obj = mp_const_none;
    if(parsed_args[transparent_color].u_obj == MP_OBJ_NULL) parsed_args[transparent_color].u_obj = mp_obj_new_int(ENGINE_NO_TRANSPARENCY_COLOR);
    if(parsed_args[fps].u_obj == MP_OBJ_NULL) parsed_args[fps].u_obj = mp_obj_new_float(30.0f);
    if(parsed_args[frame_count_x].u_obj == MP_OBJ_NULL) parsed_args[frame_count_x].u_obj = mp_obj_new_int(1);
    if(parsed_args[frame_count_y].u_obj == MP_OBJ_NULL) parsed_args[frame_count_y].u_obj = mp_obj_new_int(1);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    engine_sprite_2d_node_common_data_t *common_data = malloc(sizeof(engine_sprite_2d_node_common_data_t));
    common_data->time_at_last_animation_update_ms = millis();

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_sprite_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_SPRITE_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(inherited == false){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_sprite_2d_node_class_obj_t *sprite_2d_node = m_malloc(sizeof(engine_sprite_2d_node_class_obj_t));
        node_base->node = sprite_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_draw_obj);

        sprite_2d_node->position = parsed_args[position].u_obj;
        sprite_2d_node->texture_resource = parsed_args[texture].u_obj;
        sprite_2d_node->transparent_color = parsed_args[transparent_color].u_obj;
        sprite_2d_node->fps = parsed_args[fps].u_obj;
        sprite_2d_node->frame_count_x = parsed_args[frame_count_x].u_obj;
        sprite_2d_node->frame_count_y = parsed_args[frame_count_y].u_obj;
        sprite_2d_node->rotation = parsed_args[rotation].u_obj;
        sprite_2d_node->scale = parsed_args[scale].u_obj;
        sprite_2d_node->frame_current_x = mp_obj_new_int(0);
        sprite_2d_node->frame_current_y = mp_obj_new_int(0);
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = parsed_args[child_class].u_obj;;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_texture, parsed_args[texture].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_transparent_color, parsed_args[transparent_color].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_fps, parsed_args[fps].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_frame_count_x, parsed_args[frame_count_x].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_frame_count_y, parsed_args[frame_count_y].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_scale, parsed_args[scale].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_frame_current_x, mp_obj_new_int(0));
        mp_store_attr(node_base->node, MP_QSTR_frame_current_y, mp_obj_new_int(0));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


STATIC void sprite_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Sprite2DNode attr");

    engine_sprite_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_texture:
                destination[0] = self->texture_resource;
            break;
            case MP_QSTR_transparent_color:
                destination[0] = self->transparent_color;
            break;
            case MP_QSTR_fps:
                destination[0] = self->fps;
            break;
            case MP_QSTR_frame_count_x:
                destination[0] = self->frame_count_x;
            break;
            case MP_QSTR_frame_count_y:
                destination[0] = self->frame_count_y;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
            break;
            case MP_QSTR_frame_current_x:
                destination[0] = self->frame_current_x;
            break;
            case MP_QSTR_frame_current_y:
                destination[0] = self->frame_current_y;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_texture:
                self->texture_resource = destination[1];
            break;
            case MP_QSTR_transparent_color:
                self->transparent_color = destination[1];
            break;
            case MP_QSTR_fps:
                self->fps = destination[1];
            break;
            case MP_QSTR_frame_count_x:
                self->frame_count_x = destination[1];
            break;
            case MP_QSTR_frame_count_y:
                self->frame_count_y = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
            break;
            case MP_QSTR_frame_current_x:
                self->frame_current_x = destination[1];
            break;
            case MP_QSTR_frame_current_y:
                self->frame_current_y = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t sprite_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(sprite_2d_node_class_locals_dict, sprite_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_sprite_2d_node_class_type,
    MP_QSTR_Sprite2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, sprite_2d_node_class_new,
    print, sprite_2d_node_class_print,
    attr, sprite_2d_node_class_attr,
    locals_dict, &sprite_2d_node_class_locals_dict
);