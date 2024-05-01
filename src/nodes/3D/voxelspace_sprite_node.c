#include "voxelspace_sprite_node.h"
#include "voxelspace_node.h"

#include "nodes/node_types.h"
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
#include "draw/engine_color.h"
#include "draw/engine_shader.h"
#include "display/engine_display_common.h"


void voxelspace_sprite_node_class_draw(engine_node_base_t *sprite_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("VoxelSpaceSpriteNode: Drawing");

    engine_node_base_t *camera_node_base = camera_node;
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
    vector2_class_obj_t *sprite_scale =  voxelspace_sprite_node->scale;

    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));
    float camera_fov_half = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_fov)) * 0.5f;
    float view_distance = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_view_distance));

    uint16_t sprite_frame_count_x = mp_obj_get_int(voxelspace_sprite_node->frame_count_x);
    uint16_t sprite_frame_count_y = mp_obj_get_int(voxelspace_sprite_node->frame_count_y);
    uint16_t sprite_frame_current_x = mp_obj_get_int(voxelspace_sprite_node->frame_current_x);
    uint16_t sprite_frame_current_y = mp_obj_get_int(voxelspace_sprite_node->frame_current_y);
    bool sprite_playing = mp_obj_get_int(voxelspace_sprite_node->playing);

    color_class_obj_t *transparent_color = voxelspace_sprite_node->transparent_color;
    uint32_t spritesheet_width = sprite_texture->width;
    uint32_t spritesheet_height = sprite_texture->height;

    uint16_t *sprite_pixel_data = (uint16_t*)sprite_texture->data;

    uint32_t sprite_frame_width = spritesheet_width/sprite_frame_count_x;
    uint32_t sprite_frame_height = spritesheet_height/sprite_frame_count_y;
    uint32_t sprite_frame_abs_x = sprite_frame_width*sprite_frame_current_x;
    uint32_t sprite_frame_abs_y = sprite_frame_height*sprite_frame_current_y;
    uint32_t sprite_frame_fb_start_index = sprite_frame_abs_y * spritesheet_width + sprite_frame_abs_x;

    float view_angle = camera_rotation->x.value;
    view_angle = engine_math_clamp(view_angle, -PI/2.0f, PI/2.0f);
    view_angle = engine_math_map(view_angle, -PI/2.0f, PI/2.0f, -SCREEN_HEIGHT*2.0f, SCREEN_HEIGHT*2.0f);

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
        * opposite = soh-cah-toa -> sinO = opp/hypt -> opposite = full_length * sinf(A)
         
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
    float z = proj_adjacent / cos(camera_fov_half);

    // Is there a better way to check that the angle to the sprite
    // is within the FOV angles? IDK: TODO
    float angle_check0 = fabsf(a);
    float angle_check1 = fabsf(TWICE_PI - angle_check0);
    bool angle_in_bounds = angle_check0 < camera_fov_half || angle_check1 < camera_fov_half;


    // Check if out of view along view direct or if the angle
    // to the sprite is out of the FOV
    if(proj_adjacent > view_distance || angle_in_bounds == false){
        return;
    }

    // Figure out the y on screen
    float perspective = z * perspective_factor;
    float altitude = -sprite_position->y.value + camera_position->y.value;
    int16_t height_on_screen = (64.0f + (altitude / perspective)) + view_angle;

    float scale = 1.0f - (z * perspective_factor);
    // float scale = 1.0f;
    // ENGINE_PRINTF("%.03f\n", scale);

    float sprite_rotated_y = height_on_screen - (sprite_texture->height/2 * scale);


    // Figure out the x on screen
    float view_left_x = cosf(camera_rotation->y.value-camera_fov_half);
    float view_left_z = sinf(camera_rotation->y.value-camera_fov_half);

    float view_right_x = cosf(camera_rotation->y.value+camera_fov_half);
    float view_right_z = sinf(camera_rotation->y.value+camera_fov_half);

    float pleft_x = z * view_left_x;
    float pleft_z = z * view_left_z;

    float pright_x = z * view_right_x;
    float pright_z = z * view_right_z;

    float max_distance_between = engine_math_distance_between(pleft_x, pleft_z, pright_x, pright_z);

    pleft_x += camera_position->x.value;
    pleft_z += camera_position->z.value;

    float real_distance_between = engine_math_distance_between(pleft_x, pleft_z, sprite_position->x.value, sprite_position->z.value);
    
    float sprite_rotated_x = SCREEN_WIDTH * (real_distance_between/max_distance_between);
 

    // Decide which shader to use per-pixel
    engine_shader_t *shader = &empty_shader;
    if(sprite_opacity < 1.0f){
        shader = &opacity_shader;
    }

    engine_draw_blit(sprite_pixel_data+sprite_frame_fb_start_index,
                     floorf(sprite_rotated_x), floorf(sprite_rotated_y),
                     sprite_frame_width, sprite_frame_height,
                     spritesheet_width,
                     scale,
                     scale,
                     0.0f,
                     transparent_color->value.val,
                     sprite_opacity,
                     shader);
    
    engine_draw_pixel(0b1111100000000000, sprite_rotated_x, height_on_screen, 1.0f, shader);

    // After drawing, go to the next frame if it is time to and the animation is playing
    if(sprite_playing == 1){
        float sprite_fps = mp_obj_get_float(mp_load_attr(sprite_node_base->attr_accessor, MP_QSTR_fps));
        uint16_t sprite_period = (uint16_t)((1.0f/sprite_fps) * 1000.0f);

        uint32_t current_ms_time = millis();
        if(current_ms_time - voxelspace_sprite_node->time_at_last_animation_update_ms >= sprite_period){
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
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_add_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_remove_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_set_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_tick:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
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
            self->transparent_color = destination[1];
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
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t voxelspace_sprite_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing VoxelSpaceSpriteNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = voxelspace_sprite_node_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = voxelspace_sprite_node_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }

    return mp_const_none;
}



/*  --- doc ---
    NAME: VoxelSpaceSpriteNode
    ID: VoxelSpaceSpriteNode
    DESC: Simple 3D sprite node that can be animated or static for VoxelSpace rendering. Acts as a billboard that always faces the camera
    PARAM:  [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    PARAM:  [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    PARAM:  [type=int]                        [name=transparent_color]          [value=any 16-bit RGB565 color]
    PARAM:  [type=float]                      [name=fps]                        [value=any]
    PARAM:  [type=int]                        [name=frame_count_x]              [value=any positive integer]
    PARAM:  [type=int]                        [name=frame_count_y]              [value=any positive integer]
    PARAM:  [type=float]                      [name=rotation]                   [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    PARAM:  [type=float]                      [name=opacity]                    [value=0 ~ 1.0] 
    PARAM:  [type=boolean]                    [name=playing]                    [value=boolean]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    ATTR:   [type=int]                        [name=transparent_color]          [value=any 16-bit RGB565 color]
    ATTR:   [type=float]                      [name=fps]                        [value=any]
    ATTR:   [type=int]                        [name=frame_count_x]              [value=any positive integer]
    ATTR:   [type=int]                        [name=frame_count_y]              [value=any positive integer]
    ATTR:   [type=float]                      [name=rotation]                   [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=float]                      [name=opacity]                    [value=0 ~ 1.0]
    ATTR:   [type=boolean]                    [name=playing]                    [value=boolean]
    ATTR:   [type=int]                        [name=frame_current_x]            [value=any positive integer]
    ATTR:   [type=int]                        [name=frame_current_y]            [value=any positive integer]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
*/
mp_obj_t voxelspace_sprite_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New VoxelSpaceSpriteNode");

    // This node uses a depth buffer to be drawn correctly
    engine_display_check_depth_buffer_created();

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
        { MP_QSTR_opacity,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_playing,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, transparent_color, fps, frame_count_x, frame_count_y, rotation, scale, opacity, playing};
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

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL);
    if(parsed_args[texture].u_obj == MP_OBJ_NULL) parsed_args[texture].u_obj = mp_const_none;
    if(parsed_args[transparent_color].u_obj == MP_OBJ_NULL) parsed_args[transparent_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(ENGINE_NO_TRANSPARENCY_COLOR)});
    if(parsed_args[fps].u_obj == MP_OBJ_NULL) parsed_args[fps].u_obj = mp_obj_new_float(30.0f);
    if(parsed_args[frame_count_x].u_obj == MP_OBJ_NULL) parsed_args[frame_count_x].u_obj = mp_obj_new_int(1);
    if(parsed_args[frame_count_y].u_obj == MP_OBJ_NULL) parsed_args[frame_count_y].u_obj = mp_obj_new_int(1);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});
    if(parsed_args[opacity].u_obj == MP_OBJ_NULL) parsed_args[opacity].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[playing].u_obj == MP_OBJ_NULL) parsed_args[playing].u_obj = mp_obj_new_bool(true);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, &engine_voxelspace_sprite_node_class_type, NODE_TYPE_VOXELSPACE_SPRITE);
    engine_voxelspace_sprite_node_class_obj_t *voxelspace_sprite_node = m_malloc(sizeof(engine_voxelspace_sprite_node_class_obj_t));
    node_base->node = voxelspace_sprite_node;
    node_base->attr_accessor = node_base;

    voxelspace_sprite_node->time_at_last_animation_update_ms = millis();
    voxelspace_sprite_node->tick_cb = mp_const_none;
    voxelspace_sprite_node->position = parsed_args[position].u_obj;
    voxelspace_sprite_node->texture_resource = parsed_args[texture].u_obj;
    voxelspace_sprite_node->transparent_color = parsed_args[transparent_color].u_obj;
    voxelspace_sprite_node->fps = parsed_args[fps].u_obj;
    voxelspace_sprite_node->frame_count_x = parsed_args[frame_count_x].u_obj;
    voxelspace_sprite_node->frame_count_y = parsed_args[frame_count_y].u_obj;
    voxelspace_sprite_node->rotation = parsed_args[rotation].u_obj;
    voxelspace_sprite_node->scale = parsed_args[scale].u_obj;
    voxelspace_sprite_node->opacity = parsed_args[opacity].u_obj;
    voxelspace_sprite_node->playing = parsed_args[playing].u_obj;
    voxelspace_sprite_node->frame_current_x = mp_obj_new_int(0);
    voxelspace_sprite_node->frame_current_y = mp_obj_new_int(0);

    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);

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
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, voxelspace_sprite_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t voxelspace_sprite_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(voxelspace_sprite_node_class_locals_dict, voxelspace_sprite_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_voxelspace_sprite_node_class_type,
    MP_QSTR_VoxelSpaceSpriteNode,
    MP_TYPE_FLAG_NONE,

    make_new, voxelspace_sprite_node_class_new,
    attr, voxelspace_sprite_node_class_attr,
    locals_dict, &voxelspace_sprite_node_class_locals_dict
);