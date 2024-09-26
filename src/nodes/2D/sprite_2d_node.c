#include "sprite_2d_node.h"

#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "nodes/3D/camera_node.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "resources/engine_texture_resource.h"
#include "utility/engine_file.h"
#include "math/engine_math.h"
#include "utility/engine_time.h"
#include "draw/engine_color.h"
#include "draw/engine_shader.h"
#include "py/obj.h"


void sprite_2d_node_clamp_current_x(engine_sprite_2d_node_class_obj_t *sprite){
    uint16_t count_x = mp_obj_get_int(sprite->frame_count_x);
    uint16_t current_x = mp_obj_get_int(sprite->frame_current_x);
    if(current_x >= count_x){
        sprite->frame_current_x = mp_obj_new_int(count_x-1);
    }
}


void sprite_2d_node_clamp_current_y(engine_sprite_2d_node_class_obj_t *sprite){
    uint16_t count_y = mp_obj_get_int(sprite->frame_count_y);
    uint16_t current_y = mp_obj_get_int(sprite->frame_current_y);
    if(current_y >= count_y){
        sprite->frame_current_y = mp_obj_new_int(count_y-1);
    }
}


void sprite_2d_node_class_draw(mp_obj_t sprite_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Sprite2DNode: Drawing");

    engine_node_base_t *sprite_node_base = sprite_node_base_obj;
    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_sprite_2d_node_class_obj_t *sprite_2d_node = sprite_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float sprite_opacity = mp_obj_get_float(sprite_2d_node->opacity);
    if(engine_math_compare_floats(sprite_opacity, 0.0f)){
        return;
    }

    if(sprite_2d_node->texture_resource == mp_const_none){
        return;
    }

    texture_resource_class_obj_t *sprite_texture = sprite_2d_node->texture_resource;

    rectangle_class_obj_t *camera_viewport = camera->viewport;
    float camera_zoom = mp_obj_get_float(camera->zoom);
    float camera_opacity = mp_obj_get_float(camera->opacity);

    uint16_t sprite_frame_count_x = mp_obj_get_int(sprite_2d_node->frame_count_x);
    uint16_t sprite_frame_count_y = mp_obj_get_int(sprite_2d_node->frame_count_y);
    uint16_t sprite_frame_current_x = mp_obj_get_int(sprite_2d_node->frame_current_x);
    uint16_t sprite_frame_current_y = mp_obj_get_int(sprite_2d_node->frame_current_y);
    bool sprite_playing = mp_obj_get_int(sprite_2d_node->playing);
    bool sprite_looping = mp_obj_get_int(sprite_2d_node->loop);

    color_class_obj_t *transparent_color = sprite_2d_node->transparent_color;
    uint32_t spritesheet_width = sprite_texture->width;
    uint32_t spritesheet_height = sprite_texture->height;

    uint32_t sprite_frame_width = spritesheet_width/sprite_frame_count_x;
    uint32_t sprite_frame_height = spritesheet_height/sprite_frame_count_y;
    uint32_t sprite_frame_abs_x = sprite_frame_width*sprite_frame_current_x;
    uint32_t sprite_frame_abs_y = sprite_frame_height*sprite_frame_current_y;
    uint32_t sprite_frame_fb_start_index = sprite_frame_abs_y * spritesheet_width + sprite_frame_abs_x;

    // Get inherited properties
    engine_inheritable_2d_t inherited;
    node_base_inherit_2d(sprite_node_base, &inherited);

    if(inherited.is_camera_child == false){
        engine_camera_transform_2d(camera_node, &inherited.px, &inherited.py, &inherited.rotation);
    }else{
        camera_zoom = 1.0f;
    }

    inherited.px += camera_viewport->width/2;
    inherited.py += camera_viewport->height/2;

    sprite_opacity = inherited.opacity*camera_opacity;

    // Decide which shader to use per-pixel
    engine_shader_t *shader = NULL;
    if(sprite_opacity < 1.0f || sprite_texture->alpha_mask != 0){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    engine_draw_blit(sprite_texture, sprite_frame_fb_start_index,
                     floorf(inherited.px), floorf(inherited.py),
                     sprite_frame_width, sprite_frame_height,
                     sprite_texture->pixel_stride,
                     inherited.sx*camera_zoom,
                     inherited.sy*camera_zoom,
                    -inherited.rotation,
                     transparent_color->value,
                     sprite_opacity,
                     shader);

    // After drawing, go to the next frame if it is time to and the animation is playing
    if(sprite_playing == true){
        float sprite_fps = mp_obj_get_float(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_fps));
        uint16_t sprite_period = (uint16_t)((1.0f/sprite_fps) * 1000.0f);

        uint32_t current_ms_time = millis();
        if(millis_diff(current_ms_time, sprite_2d_node->time_at_last_animation_update_ms) >= sprite_period){
            sprite_frame_current_x++;

            // If reach end of x-axis frames, go to the next line and restart x
            if(sprite_frame_current_x >= sprite_frame_count_x){
                sprite_frame_current_x = 0;
                sprite_frame_current_y++;
            }

            bool increment_frame = true;

            // If reach end of y-axis frames, restart at x=0 and y=0
            if(sprite_frame_current_y >= sprite_frame_count_y){
                sprite_frame_current_y = 0;

                if(sprite_looping == false){
                    mp_store_attr(sprite_node_base->attr_accessor, MP_QSTR_playing, mp_obj_new_bool(false));

                    // Reached the end and looping is false, do not increment frame
                    increment_frame = false;
                }
            }

            // Update/store the current frame index only if looping
            // so that we stay on the last from when loop ends
            if(increment_frame){
                mp_store_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_x, mp_obj_new_int(sprite_frame_current_x));
                mp_store_attr(sprite_node_base->attr_accessor, MP_QSTR_frame_current_y, mp_obj_new_int(sprite_frame_current_y));
            }

            sprite_2d_node->time_at_last_animation_update_ms = millis();
        }
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool sprite_2d_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_sprite_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_texture:
            destination[0] = self->texture_resource;
            return true;
        break;
        case MP_QSTR_transparent_color:
            destination[0] = self->transparent_color;
            return true;
        break;
        case MP_QSTR_fps:
            destination[0] = self->fps;
            return true;
        break;
        case MP_QSTR_frame_count_x:
            destination[0] = self->frame_count_x;
            return true;
        break;
        case MP_QSTR_frame_count_y:
            destination[0] = self->frame_count_y;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_scale:
            destination[0] = self->scale;
            return true;
        break;
        case MP_QSTR_opacity:
            destination[0] = self->opacity;
            return true;
        break;
        case MP_QSTR_playing:
            destination[0] = self->playing;
            return true;
        break;
        case MP_QSTR_loop:
            destination[0] = self->loop;
            return true;
        break;
        case MP_QSTR_frame_current_x:
            destination[0] = self->frame_current_x;
            return true;
        break;
        case MP_QSTR_frame_current_y:
            destination[0] = self->frame_current_y;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool sprite_2d_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_sprite_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_texture:
            self->texture_resource = destination[1];
            return true;
        break;
        case MP_QSTR_transparent_color:
            self->transparent_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_fps:
            self->fps = destination[1];
            return true;
        break;
        case MP_QSTR_frame_count_x:
        {
            self->frame_count_x = destination[1];
            sprite_2d_node_clamp_current_x(self);
            return true;
        }
        break;
        case MP_QSTR_frame_count_y:
        {
            self->frame_count_y = destination[1];
            sprite_2d_node_clamp_current_y(self);
            return true;
        }
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_scale:
            self->scale = destination[1];
            return true;
        break;
        case MP_QSTR_opacity:
            self->opacity = destination[1];
            return true;
        break;
        case MP_QSTR_playing:
            self->playing = destination[1];
            return true;
        break;
        case MP_QSTR_loop:
            self->loop = destination[1];
            return true;
        break;
        case MP_QSTR_frame_current_x:
        {
            self->frame_current_x = destination[1];
            sprite_2d_node_clamp_current_x(self);
            return true;
        }
        break;
        case MP_QSTR_frame_current_y:
        {
            self->frame_current_y = destination[1];
            sprite_2d_node_clamp_current_y(self);
            return true;
        }
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t sprite_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Sprite2DNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){node_base_load_attr, sprite_2d_node_load_attr},
                          (attr_handler_func[]){node_base_store_attr, sprite_2d_node_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: Sprite2DNode
    ID: Sprite2DNode
    DESC: Simple 2D sprite node that can be animated or static
    PARAM:  [type={ref_link:Vector2}]               [name=position]                                     [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:TextureResource}]       [name=texture]                                      [value={ref_link:TextureResource}]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=transparent_color]                            [value=color]
    PARAM:  [type=float]                            [name=fps]                                          [value=any]
    PARAM:  [type=int]                              [name=frame_count_x]                                [value=any positive integer]
    PARAM:  [type=int]                              [name=frame_count_y]                                [value=any positive integer]
    PARAM:  [type=float]                            [name=rotation]                                     [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    PARAM:  [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    PARAM:  [type=boolean]                          [name=playing]                                      [value=boolean]
    PARAM:  [type=int]                              [name=layer]                                        [value=0 ~ 127]
    ATTR:   [type=function]                         [name={ref_link:add_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child_count}]                   [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy}]            [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_all}]        [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_children}]   [value=function]
    ATTR:   [type=function]                         [name={ref_link:remove_child}]                      [value=function]
    ATTR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
    ATTR:   [type={ref_link:Vector2}]               [name=position]                                     [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]               [name=global_position]                              [value={ref_link:Vector2} (read-only)]
    ATTR:   [type={ref_link:TextureResource}]       [name=texture]                                      [value={ref_link:TextureResource}]
    ATTR:   [type={ref_link:Color}|int (RGB565)]    [name=transparent_color]                            [value=color]
    ATTR:   [type=float]                            [name=fps]                                          [value=any]
    ATTR:   [type=int]                              [name=frame_count_x]                                [value=any positive integer]
    ATTR:   [type=int]                              [name=frame_count_y]                                [value=any positive integer]
    ATTR:   [type=float]                            [name=rotation]                                     [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    ATTR:   [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    ATTR:   [type=boolean]                          [name=playing]                                      [value=boolean]
    ATTR:   [type=boolean]                          [name=loop]                                         [value=boolean]
    ATTR:   [type=int]                              [name=frame_current_x]                              [value=any positive integer]
    ATTR:   [type=int]                              [name=frame_current_y]                              [value=any positive integer]
    ATTR:   [type=int]                              [name=layer]                                        [value=0 ~ 127]
    OVRR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
*/
mp_obj_t sprite_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Sprite2DNode");

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,             MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL)} },
        { MP_QSTR_texture,              MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_transparent_color,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(ENGINE_NO_TRANSPARENCY_COLOR)} },
        { MP_QSTR_fps,                  MP_ARG_OBJ, {.u_obj = mp_obj_new_float(30.0f)} },
        { MP_QSTR_frame_count_x,        MP_ARG_OBJ, {.u_obj = mp_obj_new_int(1)} },
        { MP_QSTR_frame_count_y,        MP_ARG_OBJ, {.u_obj = mp_obj_new_int(1)} },
        { MP_QSTR_rotation,             MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_scale,                MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)})} },
        { MP_QSTR_opacity,              MP_ARG_OBJ, {.u_obj = mp_obj_new_float(1.0f)} },
        { MP_QSTR_playing,              MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(true)} },
        { MP_QSTR_loop,                 MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(true)} },
        { MP_QSTR_layer,                MP_ARG_INT, {.u_int = 0} }
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, transparent_color, fps, frame_count_x, frame_count_y, rotation, scale, opacity, playing, loop, layer};
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

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_sprite_2d_node_class_type);
    node_base_init(node_base, &engine_sprite_2d_node_class_type, NODE_TYPE_SPRITE_2D, parsed_args[layer].u_int);
    engine_sprite_2d_node_class_obj_t *sprite_2d_node = m_malloc(sizeof(engine_sprite_2d_node_class_obj_t));
    node_base->node = sprite_2d_node;
    node_base->attr_accessor = node_base;

    sprite_2d_node->time_at_last_animation_update_ms = millis();
    sprite_2d_node->tick_cb = mp_const_none;
    sprite_2d_node->position = parsed_args[position].u_obj;
    sprite_2d_node->texture_resource = parsed_args[texture].u_obj;
    sprite_2d_node->transparent_color = engine_color_wrap(parsed_args[transparent_color].u_obj);
    sprite_2d_node->fps = parsed_args[fps].u_obj;
    sprite_2d_node->frame_count_x = parsed_args[frame_count_x].u_obj;
    sprite_2d_node->frame_count_y = parsed_args[frame_count_y].u_obj;
    sprite_2d_node->rotation = parsed_args[rotation].u_obj;
    sprite_2d_node->scale = parsed_args[scale].u_obj;
    sprite_2d_node->opacity = parsed_args[opacity].u_obj;
    sprite_2d_node->playing = parsed_args[playing].u_obj;
    sprite_2d_node->loop = parsed_args[loop].u_obj;
    sprite_2d_node->frame_current_x = mp_obj_new_int(0);
    sprite_2d_node->frame_current_y = mp_obj_new_int(0);

    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        node_base_set_attr_handler_default(node_instance);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            sprite_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            sprite_2d_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, sprite_2d_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t sprite_2d_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(sprite_2d_node_class_locals_dict, sprite_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_sprite_2d_node_class_type,
    MP_QSTR_Sprite2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, sprite_2d_node_class_new,
    attr, sprite_2d_node_class_attr,
    locals_dict, &sprite_2d_node_class_locals_dict
);