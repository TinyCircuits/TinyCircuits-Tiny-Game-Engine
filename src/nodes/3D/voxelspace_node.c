#include "voxelspace_node.h"

#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"
#include "math/engine_math.h"
#include "utility/linked_list.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"
#include "engine_cameras.h"
#include "draw/engine_display_draw.h"

#include <string.h>


uint8_t height_buffer[SCREEN_WIDTH];


// Class required functions
STATIC void voxelspace_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): VoxelSpaceNode");
}


STATIC mp_obj_t voxelspace_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("VoxelSpaceNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(voxelspace_node_class_tick_obj, voxelspace_node_class_tick);


void voxelspace_node_class_draw(engine_node_base_t *voxelspace_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("VoxelSpaceNode: Drawing");

    // engine_voxelspace_node_common_data_t *common_data = voxelspace_node_base->node_common_data;
    engine_node_base_t *camera_node_base = camera_node;

    texture_resource_class_obj_t *texture = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_texture);
    texture_resource_class_obj_t *heightmap = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_heightmap);

    // vector3_class_obj_t *voxelspace_rotation = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *voxelspace_position = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_position);
    float voxelspace_height_scale = mp_obj_get_float(mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_height_scale));

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    float camera_fov = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_fov));
    float camera_view_distance = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_view_distance));

    memset(height_buffer, SCREEN_HEIGHT, SCREEN_WIDTH);

    float dz = 1.0f;
    float z = 1.0f;

    while(z < camera_view_distance){
        // Instead of rotating the points by the stepped view_distance z,
        // use z as the adjacent for triangle to figure out hypotenuse
        // and then use that as the radius. This means the view distance
        // will remain the same for every FOV
        // float hypot = z / cosf(camera_rotation->y-camera_fov/2); // Not working?
        float pleft_x = z * cosf(camera_rotation->y-camera_fov/2);
        float pleft_y = z * sinf(camera_rotation->y-camera_fov/2);

        float pright_x = z * cosf(camera_rotation->y+camera_fov/2);
        float pright_y = z * sinf(camera_rotation->y+camera_fov/2);

        // float sinphi = sinf(camera_rotation->y);
        // float cosphi = cosf(camera_rotation->y);

        // float pleft_x = -cosphi*z - sinphi*z;
        // float pleft_y =  sinphi*z - cosphi*z;

        // float pright_x =  cosphi*z - sinphi*z;
        // float pright_y = -sinphi*z - cosphi*z;

        float dx = (pright_x - pleft_x) / SCREEN_WIDTH;
        float dy = (pright_y - pleft_y) / SCREEN_WIDTH;

        pleft_x += camera_position->x;
        pleft_y += camera_position->z;

        for(uint8_t i=0; i<SCREEN_WIDTH; i++){
            int32_t x = pleft_x;
            int32_t y = pleft_y;

            if((x >= voxelspace_position->x && x < voxelspace_position->x+heightmap->width) && (y >= voxelspace_position->z && y < voxelspace_position->z+heightmap->height)){
                uint32_t index = (y-voxelspace_position->z) * heightmap->width + (x-voxelspace_position->x);

                uint16_t altitude = 0;
                altitude += (heightmap->data[index] >> 0) & 0b00011111;
                altitude += (heightmap->data[index] >> 5) & 0b00111111;
                altitude += (heightmap->data[index] >> 11) & 0b00011111;

                // Use camera_rotation for on x-axis for pitch (head going in up/down in 'yes' motion)
                uint16_t height_on_screen = (-voxelspace_position->y + camera_position->y - altitude) / z * voxelspace_height_scale + (camera_rotation->x);

                // https://news.ycombinator.com/item?id=21945633
                float roll = (camera_rotation->z*(((float)i)/((float)SCREEN_WIDTH)-0.5f) + 0.5f) * SCREEN_HEIGHT / 4;

                height_on_screen += (uint16_t)roll;

                if(height_on_screen < SCREEN_HEIGHT){
                    uint8_t ipx = height_on_screen;
                    while(ipx < height_buffer[i]){
                        engine_draw_pixel(texture->data[index], i, ipx);
                        ipx++;
                    }
                }

                if(height_on_screen < height_buffer[i]){
                    height_buffer[i] = height_on_screen;
                }
            }

            pleft_x += dx;
            pleft_y += dy;
        }

        z += dz;
        dz += 0.0085f;
    }
}


/*  --- doc ---
    NAME: VoxelSpaceNode
    DESC: Node that gets rendered in a semi-3D fashion. See https://github.com/s-macke/VoxelSpace
    PARAM:  [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    PARAM:  [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    PARAM:  [type={ref_link:TextureResource}] [name=heightmap]                  [value={ref_link:TextureResource}]
    PARAM:  [type=float]                      [name=height_scale]               [value=any]
    PARAM:  [type={ref_link:Vector3}]         [name=rotation]                   [value={ref_link:Vector3}]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    ATTR:   [type={ref_link:TextureResource}] [name=texture]                    [value={ref_link:TextureResource}]
    ATTR:   [type={ref_link:TextureResource}] [name=heightmap]                  [value={ref_link:TextureResource}]
    ATTR:   [type=float]                      [name=height_scale]               [value=any]
    ATTR:   [type={ref_link:Vector3}]         [name=rotation]                   [value={ref_link:Vector3}]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
*/
mp_obj_t voxelspace_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New VoxelSpaceNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_texture,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_heightmap,            MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_height_scale,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, texture, heightmap, height_scale, rotation};
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

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL);
    if(parsed_args[texture].u_obj == MP_OBJ_NULL) parsed_args[texture].u_obj = mp_const_none;
    if(parsed_args[heightmap].u_obj == MP_OBJ_NULL) parsed_args[heightmap].u_obj = mp_const_none;
    if(parsed_args[height_scale].u_obj == MP_OBJ_NULL) parsed_args[height_scale].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = vector3_class_new(&vector3_class_type, 0, 0, NULL);

    engine_voxelspace_node_common_data_t *common_data = malloc(sizeof(engine_voxelspace_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_voxelspace_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_VOXELSPACE;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(inherited == false){        // Non-inherited (create a new object)
        engine_voxelspace_node_class_obj_t *voxelspace_node = m_malloc(sizeof(engine_voxelspace_node_class_obj_t));
        node_base->node = voxelspace_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_tick_obj);
        common_data->transform_texture_pixel_cb = MP_OBJ_NULL;
        common_data->transform_heightmap_pixel_cb = MP_OBJ_NULL;

        voxelspace_node->position = parsed_args[position].u_obj;
        voxelspace_node->texture_resource = parsed_args[texture].u_obj;
        voxelspace_node->heightmap_resource = parsed_args[heightmap].u_obj;
        voxelspace_node->height_scale = parsed_args[height_scale].u_obj;
        voxelspace_node->rotation = parsed_args[rotation].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->node = parsed_args[child_class].u_obj;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_transform_texture, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->transform_texture_pixel_cb = MP_OBJ_NULL;
        }else{                                                  // Likely found method (could be attribute)
            common_data->transform_texture_pixel_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_transform_heightmap, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->transform_heightmap_pixel_cb = MP_OBJ_NULL;
        }else{                                                  // Likely found method (could be attribute)
            common_data->transform_heightmap_pixel_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_texture, parsed_args[texture].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_heightmap, parsed_args[heightmap].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_height_scale, parsed_args[height_scale].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


STATIC void voxelspace_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing VoxelSapceNode attr");

    engine_voxelspace_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_texture:
                destination[0] = self->texture_resource;
            break;
            case MP_QSTR_heightmap:
                destination[0] = self->heightmap_resource;
            break;
            case MP_QSTR_height_scale:
                destination[0] = self->height_scale;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
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
            case MP_QSTR_heightmap:
                self->heightmap_resource = destination[1];
            break;
            case MP_QSTR_height_scale:
                self->height_scale = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t voxelspace_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(voxelspace_node_class_locals_dict, voxelspace_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_voxelspace_node_class_type,
    MP_QSTR_VoxelSpaceNode,
    MP_TYPE_FLAG_NONE,

    make_new, voxelspace_node_class_new,
    print, voxelspace_node_class_print,
    attr, voxelspace_node_class_attr,
    locals_dict, &voxelspace_node_class_locals_dict
);