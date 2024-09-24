#include "voxelspace_node.h"

#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "nodes/3D/camera_node.h"
#include "math/vector3.h"
#include "math/engine_math.h"
#include "utility/linked_list.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"
#include "draw/engine_display_draw.h"
#include "draw/engine_shader.h"
#include "py/objarray.h"

#include <string.h>


int16_t height_buffer[SCREEN_WIDTH];

// Not sure if there is a correct way to calculate this, this seems to work well
const float perspective_factor = 1.0f / SCREEN_HEIGHT_HALF;


void voxelspace_node_class_draw(mp_obj_t voxelspace_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("VoxelSpaceNode: Drawing");

    engine_node_base_t *voxelspace_node_base = voxelspace_node_base_obj;

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_voxelspace_node_class_obj_t *voxelspace_node = voxelspace_node_base->node;

    texture_resource_class_obj_t *texture = voxelspace_node->texture_resource;
    texture_resource_class_obj_t *heightmap = voxelspace_node->heightmap_resource;

    // vector3_class_obj_t *voxelspace_rotation = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *voxelspace_position = voxelspace_node->position;
    vector3_class_obj_t *voxelspace_scale = voxelspace_node->scale;
    bool repeat = mp_obj_get_int(voxelspace_node->repeat);
    bool flip = mp_obj_get_int(voxelspace_node->flip);
    float lod = mp_obj_get_float(voxelspace_node->lod);
    float curvature_angle = mp_obj_get_float(voxelspace_node->curvature);
    float thickness = mp_obj_get_float(voxelspace_node->thickness);

    vector3_class_obj_t *camera_rotation = camera->rotation;
    vector3_class_obj_t *camera_position = camera->position;
    float camera_fov_half = mp_obj_get_float(camera->fov) * 0.5f;
    float camera_view_distance = mp_obj_get_float(camera->view_distance);

    engine_shader_t *shader = engine_get_builtin_shader(EMPTY_SHADER);

    // memset(height_buffer, SCREEN_HEIGHT, SCREEN_WIDTH*2);
    for(uint16_t i=0; i<SCREEN_WIDTH; i++){
        if(flip){
            height_buffer[i] = 0;
        }else{
            height_buffer[i] = SCREEN_HEIGHT;
        }
    }

    float dz = 1.0f;
    float z = 1.0f;

    // View angle only shifts each sampled point towards the top of
    // the screen by an offset (making it look like the view is changing).
    // This does bring samples that are under the view up into view however,
    // so it does a pretty good job! Need to map radians to pixels of shift.
    //
    // Here's what certain angles should correspond to!
    //  x-axis/pitch = -90: looking directly at ground
    //  x-axis/pitch = 0:   looking directly at the horizon
    //  x-axis/pitch = 90:  looking directly at the sky
    //
    // The pitch will be clamped from -pi/2 to pi/2 for now.
    // in the future the camera could be made to look backwards
    // and upside down when the angles go out of range
    float view_angle = camera_rotation->x.value;
    view_angle = engine_math_map_clamp(view_angle, -PI/2.0f, PI/2.0f, -SCREEN_HEIGHT*2.0f, SCREEN_HEIGHT*2.0f);

    // Scales for making the terrain smaller or larger
    float inverse_x_scale = 1.0f / voxelspace_scale->x.value;
    float inverse_z_scale = 1.0f / voxelspace_scale->z.value;

    float curvature_dy = sinf(curvature_angle);
    float curvature = 0.0f;

    // https://news.ycombinator.com/item?id=21945633
    float skew_roll_line_dy = sinf(camera_rotation->z.value);
    float skew_roll_start_offset = -SCREEN_WIDTH_HALF * tanf(camera_rotation->z.value);

    float view_left_x = cosf(camera_rotation->y.value-camera_fov_half) * inverse_x_scale;
    float view_left_y = sinf(camera_rotation->y.value-camera_fov_half) * inverse_z_scale;

    float view_right_x = cosf(camera_rotation->y.value+camera_fov_half) * inverse_x_scale;
    float view_right_y = sinf(camera_rotation->y.value+camera_fov_half) * inverse_z_scale;

    // Trying to render objects in front of the camera at `camera_view_distance` units away:
    //  \-----|-----/
    //   \    |v   /
    //    \   |i  /
    //     \  |e /
    //      \ |w/
    //       \|/
    // Find the hypotenuse based on the `camera_view_distance` we
    // want to render at
    float hypot = camera_view_distance / cosf(camera_fov_half);

    while(z < hypot){
        float pleft_x = z * view_left_x;
        float pleft_y = z * view_left_y;

        float pright_x = z * view_right_x;
        float pright_y = z * view_right_y;

        float dx = (pright_x - pleft_x) * SCREEN_WIDTH_INVERSE;
        float dy = (pright_y - pleft_y) * SCREEN_WIDTH_INVERSE;

        pleft_x += camera_position->x.value * inverse_x_scale;
        pleft_y += camera_position->z.value * inverse_z_scale;

        // Cumulative offset for roll skew as the screen width is traversed
        float skew_roll_offset = skew_roll_start_offset;

        // Factor to scale certain objects/lines/distances as the render distance gets further away
        float perspective = z * perspective_factor;

        // Normalize the view along the hypot (that's what z is crawling)
        // and then scale to the max allowed in the depth buffer
        uint16_t depth = (uint16_t)((z / hypot) * UINT16_MAX);

        for(uint8_t i=0; i<SCREEN_WIDTH; i++){
            int32_t x = 0;
            int32_t y = 0;

            // Check if the terrain should render forever (repeat) or only in bounds
            if(repeat == false){
                x = (int32_t)pleft_x;
                y = (int32_t)pleft_y;

                // Only need to check bounds if repeat is not
                // true, continue for-loop if out of bounds
                if(x < voxelspace_position->x.value || x >= voxelspace_position->x.value + heightmap->width || y < voxelspace_position->z.value || y >= voxelspace_position->z.value+heightmap->height){
                    pleft_x += dx;
                    pleft_y += dy;
                    continue;
                }
            }else{
                x = (int32_t)fmodf(fabsf(pleft_x), heightmap->width);
                y = (int32_t)fmodf(fabsf(pleft_y), heightmap->height);
            }

            // Now that we know we have a position to sample, sample it
            uint32_t index = (uint32_t)((y-voxelspace_position->z.value) * heightmap->pixel_stride + (x-voxelspace_position->x.value));

            // Get each RGB channel as a float
            uint16_t heightmap_value = heightmap->get_pixel(heightmap, index, NULL);
            float r = (heightmap_value >> 0) & 0b00011111;
            float g = (heightmap_value >> 5) & 0b00111111;
            float b = (heightmap_value >> 11) & 0b00011111;

            // Change from RGB565 (already normalized to 0.0 ~ 1.0 for each channel) to grayscale 0.0 ~ 1.0: https://en.wikipedia.org/wiki/Grayscale#:~:text=Ylinear%2C-,which%20is%20given%20by,-%5B6%5D
            // Divided each channel coefficient by their respective bit resolution to avoid 3 divides
            float altitude = (0.006858f*r + 0.01135f*g + 0.00233f*b);   // <- Convert to grayscale 0.0 ~ 1.0
            if(flip == false) altitude = -altitude;                     // <- flip so that the camera is look at base/water level by default
            altitude *= voxelspace_scale->y.value;                      // Scale height from 0.0 ~ 1.0 to y-axis scale
            altitude -= voxelspace_position->y.value;                   // Apply voxelspace node translation
            altitude += camera_position->y.value;                       // Apply camera view translation

            // Use camera_rotation for on x-axis for pitch (head going in up/down in 'yes' motion)
            int16_t height_on_screen = (int16_t)(((SCREEN_HEIGHT_HALF + (altitude / perspective)) + view_angle) + curvature + skew_roll_offset);
            skew_roll_offset += skew_roll_line_dy;

            int16_t ipx = height_on_screen;

            // Clip to screen bounds so we don't draw more than needed
            if(height_on_screen >= SCREEN_HEIGHT){
                ipx = SCREEN_HEIGHT;
            }else if(height_on_screen < 0){
                ipx = -1;
            }

            if(flip){
                float drawn_thickness = 0;
                while(ipx >= height_buffer[i] && drawn_thickness < thickness){
                    if(engine_display_store_check_depth(i, ipx, depth)){
                        
                        engine_draw_pixel(texture->get_pixel(texture, index, NULL), i, ipx, 1.0f, shader);
                    }
                    ipx--;
                    drawn_thickness += perspective;
                }

                // Track the height in the buffer
                if(height_on_screen > height_buffer[i]){
                    height_buffer[i] = height_on_screen;
                }
            }else{
                float drawn_thickness = 0;
                while(ipx < height_buffer[i] && drawn_thickness < thickness){
                    if(engine_display_store_check_depth(i, ipx, depth)){
                        engine_draw_pixel(texture->get_pixel(texture, index, NULL), i, ipx, 1.0f, shader);
                    }
                    ipx++;
                    drawn_thickness += perspective;
                }

                // Track the height in the buffer
                if(height_on_screen < height_buffer[i]){
                    height_buffer[i] = height_on_screen;
                }
            }

            pleft_x += dx;
            pleft_y += dy;
        }

        z += dz;
        dz += lod;
        curvature += curvature_dy;
    }
}


/*  --- doc ---
    NAME: get_abs_height
    ID: get_abs_height
    DESC: Gets the absolute height at a position in the voxelspace node (takes position into account). If the position isn't inside the node at its current position and dimensions, returns None.
    PARAM:  [type=float]    [name=x]   [value=any]
    PARAM:  [type=float]    [name=y]   [value=any]
    RETURN: float or None
*/
static mp_obj_t voxelspace_node_class_get_abs_height(mp_obj_t self, mp_obj_t x_obj, mp_obj_t z_obj){
    engine_node_base_t *node_base = self;
    engine_voxelspace_node_class_obj_t *voxelspace = node_base->node;
    vector3_class_obj_t *voxelspace_position = voxelspace->position;
    vector3_class_obj_t *voxelspace_scale = voxelspace->scale;
    texture_resource_class_obj_t *heightmap = voxelspace->heightmap_resource;

    // Scales for making the terrain smaller or larger
    float inverse_x_scale = 1.0f / voxelspace_scale->x.value;
    float inverse_z_scale = 1.0f / voxelspace_scale->z.value;

    bool repeat = mp_obj_get_int(voxelspace->repeat);
    float x = mp_obj_get_float(x_obj) * inverse_x_scale;
    float z = mp_obj_get_float(z_obj) * inverse_z_scale;

    if(repeat == true){
        x = fmodf(fabsf(x), heightmap->width);
        z = fmodf(fabsf(z), heightmap->height);
    }

    // Only need to check bounds if repeat is not true
    if(repeat == true || ((x >= voxelspace_position->x.value && x < voxelspace_position->x.value + heightmap->width) && (z >= voxelspace_position->z.value && z < voxelspace_position->z.value+heightmap->height))){
        uint32_t index = (uint32_t)(((int32_t)z-voxelspace_position->z.value) * heightmap->pixel_stride + ((int32_t)x-voxelspace_position->x.value));

        // Get each RGB channel as a float
        uint16_t heightmap_value = heightmap->get_pixel(heightmap, index, NULL);
        float r = (heightmap_value >> 0) & 0b00011111;
        float g = (heightmap_value >> 5) & 0b00111111;
        float b = (heightmap_value >> 11) & 0b00011111;

        // Change from RGB565 (already normalized to 0.0 ~ 1.0 for each channel) to grayscale 0.0 ~ 1.0: https://en.wikipedia.org/wiki/Grayscale#:~:text=Ylinear%2C-,which%20is%20given%20by,-%5B6%5D
        // Divided each channel coefficient by their respective bit resolution to avoid 3 divides
        float altitude = (0.006858f*r + 0.01135f*g + 0.00233f*b);   // <- Convert to grayscale 0.0 ~ 1.0

        bool flip = mp_obj_get_int(voxelspace->flip);

        if(flip == false) altitude = -altitude;     // <- flip so that the camera is look at base/water level by default
        altitude *= voxelspace_scale->y.value;      // Scale height from 0.0 ~ 1.0 to y-axis scale
        altitude -= voxelspace_position->y.value;   // Apply voxelspace node translation

        return mp_obj_new_float(-altitude);
    }else{
        return mp_const_none;
    }


    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(voxelspace_node_class_get_abs_height_obj, voxelspace_node_class_get_abs_height);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool voxelspace_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_voxelspace_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_get_abs_height:
            destination[0] = MP_OBJ_FROM_PTR(&voxelspace_node_class_get_abs_height_obj);
            destination[1] = self_node_base;
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
        case MP_QSTR_heightmap:
            destination[0] = self->heightmap_resource;
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
        case MP_QSTR_repeat:
            destination[0] = self->repeat;
            return true;
        break;
        case MP_QSTR_flip:
            destination[0] = self->flip;
            return true;
        break;
        case MP_QSTR_lod:
            destination[0] = self->lod;
            return true;
        break;
        case MP_QSTR_curvature:
            destination[0] = self->curvature;
            return true;
        break;
        case MP_QSTR_thickness:
            destination[0] = self->thickness;
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
bool voxelspace_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_voxelspace_node_class_obj_t *self = self_node_base->node;

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
        case MP_QSTR_heightmap:
            self->heightmap_resource = destination[1];
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
        case MP_QSTR_repeat:
            self->repeat = destination[1];
            return true;
        break;
        case MP_QSTR_flip:
            self->flip = destination[1];
            return true;
        break;
        case MP_QSTR_lod:
            self->lod = destination[1];
            return true;
        break;
        case MP_QSTR_curvature:
            self->curvature = destination[1];
            return true;
        break;
        case MP_QSTR_thickness:
            self->thickness = destination[1];
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


static mp_attr_fun_t voxelspace_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing VoxelspaceNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){voxelspace_node_load_attr, node_base_load_attr},
                          (attr_handler_func[]){voxelspace_node_store_attr, node_base_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: VoxelSpaceNode
    ID: VoxelSpaceNode
    DESC: Node that gets rendered in a semi-3D fashion. See https://github.com/s-macke/VoxelSpace. If a camera is at 0,0,0 with rotation 0,0,0 and a voxelspace node is at 0,0,0 with rotation 0,0,0, the camera will be at a corner of the node where forward is following the node in the +x-axis direction and right is following the node in the +y-axis direction. If the voxelspace y-axis scale is set to 25 then full white pixels will be at 25 in height in world space as long as voxelsapce node's position is 0,0,0. Currently, camera the x-axis rotation is pitch (clamped and mapped to -90 or -pi/2 (ground) to 90 or pi/2 (sky)), y-axis rotation is yaw, and the z-axis rotation is a fake roll.
    PARAM:  [type={ref_link:Vector3}]         [name=position]                                   [value={ref_link:Vector3}]
    PARAM:  [type={ref_link:TextureResource}] [name=texture]                                    [value={ref_link:TextureResource}]
    PARAM:  [type={ref_link:TextureResource}] [name=heightmap]                                  [value={ref_link:TextureResource}]
    PARAM:  [type=float]                      [name=height_scale]                               [value=any]
    PARAM:  [type={ref_link:Vector3}]         [name=rotation]                                   [value={ref_link:Vector3}]
    PARAM:  [type=int]                        [name=layer]                                      [value=0 ~ 127]
    ATTR:   [type=function]                   [name={ref_link:add_child}]                       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]                       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child_count}]                 [value=function]
    ATTR:   [type=function]                   [name={ref_link:node_base_mark_destroy}]          [value=function]
    ATTR:   [type=function]                   [name={ref_link:node_base_mark_destroy_all}]      [value=function]
    ATTR:   [type=function]                   [name={ref_link:node_base_mark_destroy_children}] [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]                    [value=function]
    ATTR:   [type=function]                   [name={ref_link:tick}]                            [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_abs_height}]                  [value=function]
    ATTR:   [type={ref_link:Vector3}]         [name=position]                                   [value={ref_link:Vector3}]
    ATTR:   [type={ref_link:TextureResource}] [name=texture]                                    [value={ref_link:TextureResource}]
    ATTR:   [type={ref_link:TextureResource}] [name=heightmap]                                  [value={ref_link:TextureResource}]
    ATTR:   [type={ref_link:Vector3}]         [name=rotation]                                   [value={ref_link:Vector3}]
    ATTR:   [type={ref_link:Vector3}]         [name=scale]                                      [value=any (x-axis makes terrain wider (default: 1.0), y-axis makes terrain taller/shorter (default: 10.0, this means the min height will be 0.0 and the max 10.0 if the node position is 0,0,0), and z-axis makes terrain longer (default: 1.0))]
    ATTR:   [type=boolean]                    [name=repeat]                                     [value=True or False (if True, repeats the terrain forever in all directions, default: False)]
    ATTR:   [type=boolean]                    [name=flip]                                       [value=True or False (flips drawing upsidedown if True and normal if False (default))]
    ATTR:   [type=float]                      [name=lod]                                        [value=any (stand for Level Of Detail and affects the quality/number of samples as the view is rendered at further and further distances, default: 0.0085)]
    ATTR:   [type=float]                      [name=curvature]                                  [value=any (radians, defines how much the terrain curves as the render distance increases, default: 0.0)]
    ATTR:   [type=float]                      [name=thickness]                                  [value=any (defines how thick the terrain should look if you can see its sides, default: 128.0)]
    ATTR:   [type=int]                        [name=layer]                                      [value=0 ~ 127]
    OVRR:   [type=function]                   [name={ref_link:tick}]                            [value=function]
*/
mp_obj_t voxelspace_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New VoxelSpaceNode");

    // This node uses a depth buffer to be drawn correctly
    engine_display_check_depth_buffer_created();

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL)} },
        { MP_QSTR_texture,      MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_heightmap,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL)} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = vector3_class_new(&vector3_class_type, 3, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(10.0f), mp_obj_new_float(1.0f)})} },
        { MP_QSTR_repeat,       MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(false)} },
        { MP_QSTR_flip,         MP_ARG_OBJ, {.u_obj = mp_obj_new_bool(false)} },
        { MP_QSTR_lod,          MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0085f)} },
        { MP_QSTR_curvature,    MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_thickness,    MP_ARG_OBJ, {.u_obj = mp_obj_new_float(128.0f)} },
        { MP_QSTR_layer,        MP_ARG_INT, {.u_int = 0} }
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, heightmap, rotation, scale, repeat, flip, lod, curvature, thickness, layer};
    bool inherited = false;

    // If there is one positional argument and it isn't the first
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector3_class_type){
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
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_voxelspace_node_class_type);
    node_base_init(node_base, &engine_voxelspace_node_class_type, NODE_TYPE_VOXELSPACE, parsed_args[layer].u_int);
    engine_voxelspace_node_class_obj_t *voxelspace_node = m_malloc(sizeof(engine_voxelspace_node_class_obj_t));
    node_base->node = voxelspace_node;
    node_base->attr_accessor = node_base;

    voxelspace_node->tick_cb = mp_const_none;
    voxelspace_node->position = parsed_args[position].u_obj;
    voxelspace_node->texture_resource = parsed_args[texture].u_obj;
    voxelspace_node->heightmap_resource = parsed_args[heightmap].u_obj;
    voxelspace_node->rotation = parsed_args[rotation].u_obj;
    voxelspace_node->scale = parsed_args[scale].u_obj;
    voxelspace_node->repeat = parsed_args[repeat].u_obj;
    voxelspace_node->flip = parsed_args[flip].u_obj;
    voxelspace_node->lod = parsed_args[lod].u_obj;
    voxelspace_node->curvature = parsed_args[curvature].u_obj;
    voxelspace_node->thickness = parsed_args[thickness].u_obj;

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
            voxelspace_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            voxelspace_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, voxelspace_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t voxelspace_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(voxelspace_node_class_locals_dict, voxelspace_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_voxelspace_node_class_type,
    MP_QSTR_VoxelSpaceNode,
    MP_TYPE_FLAG_NONE,

    make_new, voxelspace_node_class_new,
    attr, voxelspace_node_class_attr,
    locals_dict, &voxelspace_node_class_locals_dict
);