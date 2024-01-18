#include "voxelspace_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"
#include "math/engine_math.h"
#include "utility/linked_list.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"
#include "engine_cameras.h"
#include "draw/engine_display_draw.h"


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


STATIC mp_obj_t voxelspace_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("VoxelSpaceNode: Drawing");

    // Decode and store properties about the rectangle and camera nodes
    engine_node_base_t *voxelspace_node_base = self_in;
    // engine_voxelspace_node_common_data_t *common_data = voxelspace_node_base->node_common_data;
    engine_node_base_t *camera_node_base = camera_node;

    texture_resource_class_obj_t *texture = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_texture_resource);
    texture_resource_class_obj_t *heightmap = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_heightmap_resource);

    // vector3_class_obj_t *voxelspace_rotation = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *voxelspace_position = mp_load_attr(voxelspace_node_base->attr_accessor, MP_QSTR_position);

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    // rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);

    const float view_horizon = 45.0f;
    const float scale_height = 30.0f;
    const float view_distance = 24.0f;

    float sinang = sin(camera_rotation->y);
    float cosang = cos(camera_rotation->y);

    float z = view_distance;

    while(z > 0.0f){
        float pleft_x = -sinang * z - cosang * z;
        float pleft_y =  cosang * z - sinang * z;

        float pright_x =  sinang * z - cosang * z;
        float pright_y = -cosang * z - sinang * z;

        float dx = (pright_x - pleft_x) / SCREEN_WIDTH;
        float dy = (pright_y - pleft_y) / SCREEN_WIDTH;

        pleft_x += camera_position->x + voxelspace_position->x;
        pleft_y += camera_position->z + voxelspace_position->y;

        for(uint8_t i=0; i<SCREEN_WIDTH; i++){
            int32_t x = pleft_x;
            int32_t y = pleft_y;

            if((x >= 0 && x < heightmap->width) && (y >= 0 && y < heightmap->height)){
                uint32_t index = y * heightmap->width + x;

                uint16_t altitude = 0;
                altitude += (heightmap->data[index] >> 0) & 0b00011111;
                altitude += (heightmap->data[index] >> 5) & 0b00111111;
                altitude += (heightmap->data[index] >> 11) & 0b00011111;

                uint16_t height_on_screen = (camera_position->y - altitude) / z * scale_height + (view_horizon + camera_rotation->x);

                if(height_on_screen < SCREEN_HEIGHT){
                    uint8_t ipx = height_on_screen;
                    while(ipx < SCREEN_HEIGHT){
                        engine_draw_pixel(texture->data[index], i, ipx);
                        ipx++;
                    }
                }
            }

            pleft_x += dx;
            pleft_y += dy;
        }

        z -= 1.0f;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(voxelspace_node_class_draw_obj, voxelspace_node_class_draw);


mp_obj_t voxelspace_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New VoxelSpaceNode");
    
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


    if(n_args == 2){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_voxelspace_node_class_obj_t *voxelspace_node = m_malloc(sizeof(engine_voxelspace_node_class_obj_t));
        node_base->node = voxelspace_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_draw_obj);
        common_data->transform_texture_pixel_cb = MP_OBJ_NULL;
        common_data->transform_heightmap_pixel_cb = MP_OBJ_NULL;

        voxelspace_node->position = vector3_class_new(&vector3_class_type, 0, 0, NULL);
        voxelspace_node->rotation = vector3_class_new(&vector3_class_type, 0, 0, NULL);
        voxelspace_node->texture_resource = args[0];
        voxelspace_node->heightmap_resource = args[1];
    }else if(n_args == 3){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&voxelspace_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
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

        mp_store_attr(node_base->node, MP_QSTR_position, vector3_class_new(&vector3_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_rotation, vector3_class_new(&vector3_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_texture_resource, args[1]);
        mp_store_attr(node_base->node, MP_QSTR_heightmap_resource, args[2]);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Too many arguments passed to VoxelSpaceNode constructor!"));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
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
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_texture_resource:
                destination[0] = self->texture_resource;
            break;
            case MP_QSTR_heightmap_resource:
                destination[0] = self->heightmap_resource;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_texture_resource:
                self->texture_resource = destination[1];
            break;
            case MP_QSTR_heightmap_resource:
                self->heightmap_resource = destination[1];
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