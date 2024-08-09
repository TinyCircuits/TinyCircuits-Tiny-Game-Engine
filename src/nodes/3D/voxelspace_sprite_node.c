#include "voxelspace_sprite_node.h"
#include "voxelspace_node.h"

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
#include "display/engine_display_common.h"


extern const float perspective_factor;


void voxelspace_sprite_node_class_draw(mp_obj_t sprite_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("VoxelSpaceSpriteNode: Drawing");

    engine_node_base_t *sprite_node_base = sprite_node_base_obj;

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_voxelspace_sprite_node_class_obj_t *voxelspace_sprite_node = sprite_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float sprite_opacity = mp_obj_get_float(voxelspace_sprite_node->opacity);
    if(engine_math_compare_floats(sprite_opacity, 0.0f)){
        return;
    }

    if(voxelspace_sprite_node->texture_resource == mp_const_none){
        return;
    }

    texture_resource_class_obj_t *sprite_texture = voxelspace_sprite_node->texture_resource;
    vector3_class_obj_t *sprite_position = voxelspace_sprite_node->position;
    float sprite_rotation = mp_obj_get_float(voxelspace_sprite_node->rotation);
    vector2_class_obj_t *sprite_scale =  voxelspace_sprite_node->scale;
    bool sprite_fov_distort = mp_obj_get_int(voxelspace_sprite_node->fov_distort);
    vector2_class_obj_t *sprite_texture_offset =  voxelspace_sprite_node->texture_offset;

    vector3_class_obj_t *camera_position = camera->position;
    vector3_class_obj_t *camera_rotation = camera->rotation;
    float camera_fov_half = mp_obj_get_float(camera->fov) * 0.5f;
    float view_distance = mp_obj_get_float(camera->view_distance);

    uint16_t sprite_frame_count_x = mp_obj_get_int(voxelspace_sprite_node->frame_count_x);
    uint16_t sprite_frame_count_y = mp_obj_get_int(voxelspace_sprite_node->frame_count_y);
    uint16_t sprite_frame_current_x = mp_obj_get_int(voxelspace_sprite_node->frame_current_x);
    uint16_t sprite_frame_current_y = mp_obj_get_int(voxelspace_sprite_node->frame_current_y);
    bool sprite_playing = mp_obj_get_int(voxelspace_sprite_node->playing);

    color_class_obj_t *transparent_color = voxelspace_sprite_node->transparent_color;
    uint32_t spritesheet_width = sprite_texture->width;
    uint32_t spritesheet_height = sprite_texture->height;

    uint16_t *sprite_pixel_data = ((mp_obj_array_t*)sprite_texture->data)->items;

    int32_t sprite_frame_width = spritesheet_width/sprite_frame_count_x;
    int32_t sprite_frame_height = spritesheet_height/sprite_frame_count_y;
    uint32_t sprite_frame_abs_x = sprite_frame_width*sprite_frame_current_x;
    uint32_t sprite_frame_abs_y = sprite_frame_height*sprite_frame_current_y;
    uint32_t sprite_frame_fb_start_index = sprite_frame_abs_y * spritesheet_width + sprite_frame_abs_x;

    float view_angle = camera_rotation->x.value;
    view_angle = engine_math_map_clamp(view_angle, -PI/2.0f, PI/2.0f, -SCREEN_HEIGHT*2.0f, SCREEN_HEIGHT*2.0f);

    // Will need to pick a z that is valid. The z should be
    // such that it is picked on a line between 1.0 and
    // `camera_view_distance / cosf(camera_fov_half)` at
    // an angle camera_fov_half from the view direction line
    // that has a length of `camera_view_distance`:
    /*

                   opposite
            \---------------------|                     /  ---
             \         P          |                    /    |
     ---     L?--------*----------|-------------------?R    |
       \       \       |         p|                  /      |
        \       \       \        r|                 /       | v
         \      f\       |       o|                /        | i
          \      u\      |       j|a              /         | e
           \      l\      \      -|d             /          | w
            \      l\      |     a|j            /           |
             \      _\     |     d|a           /            | d
              \      l\   D \    j|c          /             | i
              z\      e\     |   a|e         /              | s
                \      n\    |   c|n        /               | t
                 \      g\    \  e|t       /                | a
                  \      t\    | n|       /                 | n
                   \      h\   | t|      /                  | c
                    \       \  \  |     /                   | e
                     \       \  | |    /                    |
                      \       \ |a|   /                     |
                       \       \A\|  /                      |
                        \       \ | /                      ---
                        ---       *
                                  C

        Based on the above diagram, we know:
        * P's location in absolute space
        * C'c location in absolute space
        * Angle A = camera_fov_half
        * adjacent = view_distance (is given/provided)
        * full_length = view_distance / cosf(A) [soh-cah-toa -> cah -> cosO = addj/hypt -> full_length = view_distance / cosA]
        * opposite = soh-cah-toa -> sinO = opp/hypt -> opposite = full_length * sinf(camera_fov_half)

        To do the correct projection we need to find a distance along 'full_length' that we can
        use to find L and R based on the voxelspace calculations below. We need to choose this
        distance based on the point P relative to C. Once we have L and R we can see how far along
        P is from L to R and map that to 0 to SCREEN_WIDTH

        To find the distance z to use:

        cah -> cosf(a) = proj_adjacent / D -> [proj_adjacent = D * cosf(a)]

        cah -> cosf(A) = proj_adjacent / z -> [z = proj_adjacent / cosf(camera_fov_half)]
    */
    float D = engine_math_distance_between(camera_position->x.value, camera_position->z.value, sprite_position->x.value, sprite_position->z.value);
    float angle = camera_rotation->y.value;

    float a = engine_math_angle_between(camera_position->x.value, camera_position->z.value, sprite_position->x.value, sprite_position->z.value) - angle;
    float proj_adjacent = D * cosf(a);
    float z = proj_adjacent / cosf(camera_fov_half);

    // The z we calculate here can vary from 0 to view_distance / cosf(A),
    // Normalize the view along the full_length (that's what z is crawling)
    // and then scale to the max allowed in the depth buffer
    float full_length = view_distance / cosf(camera_fov_half);
    uint16_t depth = (uint16_t)((z / full_length) * UINT16_MAX);

    // Check if out of view along view direct or if the angle
    // to the sprite is out of the FOV
    if(proj_adjacent > view_distance){
        return;
    }

    // Figure out the perspective
    float perspective = z * perspective_factor;
    float inverse_perspective = 1.0f / perspective;

    float view_left_x = cosf(camera_rotation->y.value-camera_fov_half);
    float view_left_z = sinf(camera_rotation->y.value-camera_fov_half);

    float view_right_x = cosf(camera_rotation->y.value+camera_fov_half);
    float view_right_z = sinf(camera_rotation->y.value+camera_fov_half);

    float pleft_x = z * view_left_x + camera_position->x.value;
    float pleft_z = z * view_left_z + camera_position->z.value;

    float pright_x = z * view_right_x + camera_position->x.value;
    float pright_z = z * view_right_z + camera_position->z.value;

    // https://www.sunshine2k.de/coding/java/PointOnLine/PointOnLine.html
    // Project the sprite position onto the line formed
    // py the points on the view area. This way positions
    // outside of teh view can be found to that sprites
    // don't just disappear at the edge
    float sx = sprite_position->x.value;
    float sz = sprite_position->z.value;

    float e1x = pright_x - pleft_x;
    float e1z = pright_z - pleft_z;

    float e2x = sx - pleft_x;
    float e2z = sz - pleft_z;

    float ax = view_right_x + camera_position->x.value;
    float az = view_right_z + camera_position->z.value;

    float bx = view_left_x + camera_position->x.value;
    float bz = view_left_z + camera_position->z.value;

    // Do 2D cross product and test sign to see which
    // side of the view the sprite wants to render on:
    // behind or in front
    // https://stackoverflow.com/a/3461533
    float cross = (bx - ax)*(sz - az) - (bz - az)*(sx - ax);
    if(cross < 0.0f){
        return;
    }

    float max = engine_math_dot_product(e1x, e1z, e1x, e1z);
    float value = engine_math_dot_product(e1x, e1z, e2x, e2z);
    float sprite_rotated_x = ((value/max) * SCREEN_WIDTH);

    // Figure out the scale
    // This scales everything so that if you're one projected
    // to view distance axis away, the sprite will show up as
    // its own full height (not the screen)
    // float aspect = (1.0f / (((1.0f) / cos(camera_fov_half)) * perspective_factor))/2.0f;
    // float scale_x = inverse_perspective / aspect * sprite_scale->x.value;
    // float scale_y = inverse_perspective / aspect * sprite_scale->y.value;

    // Figure out the scale to due fov shift (not the best): TODO
    // float opposite = full_length * sinf(camera_fov_half) * 2.0f;
    // float opposite = full_length * sinf(camera_fov_half) * 2.0f;
    // float fov_x_scale = SCREEN_WIDTH/opposite;
    // float fov_x_scale = SCREEN_WIDTH/max_distance_between;
    float fov_x_scale = 1.0f;
    if(sprite_fov_distort == true){
        fov_x_scale = PI/4.0f / (camera_fov_half*2.0f) * 0.707f;
    }

    float scale_x = inverse_perspective * sprite_scale->x.value * fov_x_scale;
    float scale_y = inverse_perspective * sprite_scale->y.value;

    // Figure out the y on screen
    float altitude = -sprite_position->y.value + camera_position->y.value;
    int16_t height_on_screen = (int16_t)((SCREEN_HEIGHT_HALF + (altitude * inverse_perspective)) + view_angle);

    // Apply x and y
    sprite_rotated_x += (sprite_texture_offset->x.value * scale_x);
    float sprite_rotated_y = height_on_screen - (sprite_texture_offset->y.value * scale_y);

    // Decide which shader to use per-pixel
    engine_shader_t *shader = NULL;
    if(sprite_opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    engine_draw_blit_depth(sprite_pixel_data+sprite_frame_fb_start_index,
                     floorf(sprite_rotated_x), floorf(sprite_rotated_y),
                     sprite_frame_width, sprite_frame_height,
                     spritesheet_width,
                     scale_x,
                     scale_y,
                     -sprite_rotation,
                     transparent_color->value,
                     sprite_opacity,
                     depth,
                     shader);

    // engine_draw_pixel(0b1111100000000000, floorf(((value/max) * SCREEN_WIDTH)), floorf(sprite_rotated_y), 1.0f, shader);

    // After drawing, go to the next frame if it is time to and the animation is playing
    if(sprite_playing == 1){
        float sprite_fps = mp_obj_get_float(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_fps));
        uint16_t sprite_period = (uint16_t)((1.0f/sprite_fps) * 1000.0f);

        uint32_t current_ms_time = millis();
        if(millis_diff(current_ms_time, voxelspace_sprite_node->time_at_last_animation_update_ms) >= sprite_period){
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
            voxelspace_sprite_node->time_at_last_animation_update_ms = millis();
        }
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool voxelspace_sprite_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_voxelspace_sprite_node_class_obj_t *self = self_node_base->node;

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
        case MP_QSTR_frame_current_x:
            destination[0] = self->frame_current_x;
            return true;
        break;
        case MP_QSTR_frame_current_y:
            destination[0] = self->frame_current_y;
            return true;
        break;
        case MP_QSTR_fov_distort:
            destination[0] = self->fov_distort;
            return true;
        break;
        case MP_QSTR_texture_offset:
            destination[0] = self->texture_offset;
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool voxelspace_sprite_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_voxelspace_sprite_node_class_obj_t *self = self_node_base->node;

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
            self->frame_count_x = destination[1];
            return true;
        break;
        case MP_QSTR_frame_count_y:
            self->frame_count_y = destination[1];
            return true;
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
        case MP_QSTR_frame_current_x:
            self->frame_current_x = destination[1];
            return true;
        break;
        case MP_QSTR_frame_current_y:
            self->frame_current_y = destination[1];
            return true;
        break;
        case MP_QSTR_fov_distort:
            self->fov_distort = destination[1];
            return true;
        break;
        case MP_QSTR_texture_offset:
            self->texture_offset = destination[1];
            return true;
        break;
        case MP_QSTR_global_position:
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: `global_position` is not supported on this node yet!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t voxelspace_sprite_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing VoxelspaceNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){voxelspace_sprite_node_load_attr, node_base_load_attr},
                          (attr_handler_func[]){voxelspace_sprite_node_store_attr, node_base_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: VoxelSpaceSpriteNode
    ID: VoxelSpaceSpriteNode
    DESC: Simple 3D sprite node that can be animated or static for VoxelSpace rendering. Acts as a billboard that always faces the camera
    PARAM:  [type={ref_link:Vector3}]               [name=position]                                     [value={ref_link:Vector3}]
    PARAM:  [type={ref_link:TextureResource}]       [name=texture]                                      [value={ref_link:TextureResource}]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=transparent_color]                            [value=color]
    PARAM:  [type=float]                            [name=fps]                                          [value=any]
    PARAM:  [type=int]                              [name=frame_count_x]                                [value=any positive integer]
    PARAM:  [type=int]                              [name=frame_count_y]                                [value=any positive integer]
    PARAM:  [type=float]                            [name=rotation]                                     [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    PARAM:  [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    PARAM:  [type=boolean]                          [name=playing]                                      [value=boolean]
    PARAM:  [type=boolean]                          [name=fov_distort]                                  [value=boolean (True means the sprite will be scaled by the FOV (TODO: review implementation, not perfect) and False means it will not be distorted, default: True)]
    PARAM:  [type={ref_link:Vector2}]               [name=texture_offset]                               [value={ref_link:Vector2} (local offset of the texture at the rendered origin. Sprites render at center/origin by default, use this to shift them)]
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
    ATTR:   [type={ref_link:TextureResource}]       [name=texture]                                      [value={ref_link:TextureResource}]
    ATTR:   [type={ref_link:Color}|int (RGB565)]    [name=transparent_color]                            [value=color]
    ATTR:   [type=float]                            [name=fps]                                          [value=any]
    ATTR:   [type=int]                              [name=frame_count_x]                                [value=any positive integer]
    ATTR:   [type=int]                              [name=frame_count_y]                                [value=any positive integer]
    ATTR:   [type=float]                            [name=rotation]                                     [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    ATTR:   [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    ATTR:   [type=boolean]                          [name=playing]                                      [value=boolean]
    ATTR:   [type=int]                              [name=frame_current_x]                              [value=any positive integer]
    ATTR:   [type=int]                              [name=frame_current_y]                              [value=any positive integer]
    ATTR:   [type=boolean]                          [name=fov_distort]                                  [value=boolean (True means the sprite will be scaled by the FOV (TODO: review implementation, not perfect) and False means it will not be distorted, default: True)]
    ATTR:   [type={ref_link:Vector2}]               [name=texture_offset]                               [value={ref_link:Vector2} (local offset of the texture at the rendered origin. Sprites render at center/origin by default, use this to shift them)]
    ATTR:   [type=int]                              [name=layer]                                        [value=0 ~ 127]
    OVRR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
*/
mp_obj_t voxelspace_sprite_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New VoxelSpaceSpriteNode");

    // This node uses a depth buffer to be drawn correctly
    engine_display_check_depth_buffer_created();

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,             MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL)} },
        { MP_QSTR_texture,              MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_transparent_color,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(ENGINE_NO_TRANSPARENCY_COLOR)} },
        { MP_QSTR_fps,                  MP_ARG_OBJ, {.u_obj = mp_obj_new_float(30.0f)} },
        { MP_QSTR_frame_count_x,        MP_ARG_OBJ, {.u_obj = mp_obj_new_int(1)} },
        { MP_QSTR_frame_count_y,        MP_ARG_OBJ, {.u_obj = mp_obj_new_int(1)} },
        { MP_QSTR_rotation,             MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_scale,                MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)})} },
        { MP_QSTR_opacity,              MP_ARG_OBJ, {.u_obj = mp_obj_new_float(1.0f)} },
        { MP_QSTR_playing,              MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(true)} },
        { MP_QSTR_fov_distort,          MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(true)} },
        { MP_QSTR_texture_offset,       MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL)} },
        { MP_QSTR_layer,                MP_ARG_INT, {.u_int = 0} }
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, transparent_color, fps, frame_count_x, frame_count_y, rotation, scale, opacity, playing, fov_distort, texture_offset, layer};
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
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_voxelspace_sprite_node_class_type);
    node_base_init(node_base, &engine_voxelspace_sprite_node_class_type, NODE_TYPE_VOXELSPACE_SPRITE, parsed_args[layer].u_int);
    engine_voxelspace_sprite_node_class_obj_t *voxelspace_sprite_node = m_malloc(sizeof(engine_voxelspace_sprite_node_class_obj_t));
    node_base->node = voxelspace_sprite_node;
    node_base->attr_accessor = node_base;

    voxelspace_sprite_node->time_at_last_animation_update_ms = millis();
    voxelspace_sprite_node->tick_cb = mp_const_none;
    voxelspace_sprite_node->position = parsed_args[position].u_obj;
    voxelspace_sprite_node->texture_resource = parsed_args[texture].u_obj;
    voxelspace_sprite_node->transparent_color = engine_color_wrap(parsed_args[transparent_color].u_obj);
    voxelspace_sprite_node->fps = parsed_args[fps].u_obj;
    voxelspace_sprite_node->frame_count_x = parsed_args[frame_count_x].u_obj;
    voxelspace_sprite_node->frame_count_y = parsed_args[frame_count_y].u_obj;
    voxelspace_sprite_node->rotation = parsed_args[rotation].u_obj;
    voxelspace_sprite_node->scale = parsed_args[scale].u_obj;
    voxelspace_sprite_node->opacity = parsed_args[opacity].u_obj;
    voxelspace_sprite_node->playing = parsed_args[playing].u_obj;
    voxelspace_sprite_node->frame_current_x = mp_obj_new_int(0);
    voxelspace_sprite_node->frame_current_y = mp_obj_new_int(0);
    voxelspace_sprite_node->fov_distort = parsed_args[fov_distort].u_obj;
    voxelspace_sprite_node->texture_offset = parsed_args[texture_offset].u_obj;

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
            voxelspace_sprite_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            voxelspace_sprite_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, voxelspace_sprite_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t voxelspace_sprite_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(voxelspace_sprite_node_class_locals_dict, voxelspace_sprite_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_voxelspace_sprite_node_class_type,
    MP_QSTR_VoxelSpaceSpriteNode,
    MP_TYPE_FLAG_NONE,

    make_new, voxelspace_sprite_node_class_new,
    attr, voxelspace_sprite_node_class_attr,
    locals_dict, &voxelspace_sprite_node_class_locals_dict
);