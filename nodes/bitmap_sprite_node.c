#include "bitmap_sprite_node.h"

#include "node_types.h"
#include "nodes/camera_node.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"
#include "math/vector3.h"
#include "display/engine_display_draw.h"

// Class required functions
STATIC void bitmap_sprite_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): BitmapSpriteNode");
}

mp_obj_t bitmap_sprite_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New BitmapSpriteNode");
    
    // Check that there's an argument that's hopefully a reference to the inheriting subclass
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_bitmap_sprite_node_class_obj_t *self = m_new_obj_with_finaliser(engine_bitmap_sprite_node_class_obj_t);
    self->base.type = &engine_bitmap_sprite_node_class_type;
    self->node_base.layer = 0;
    self->node_base.type = NODE_TYPE_BITMAP_SPRITE;
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);
    node_base_set_if_visible(&self->node_base, true);
    node_base_set_if_disabled(&self->node_base, false);
    node_base_set_if_just_added(&self->node_base, true);

    // Cache lookup results of 'tick()' and 'draw()' functions on this
    // node instance so that the main engine loop can call it quickly
    mp_load_method(MP_OBJ_TO_PTR(args[0]), MP_QSTR_tick, self->tick_dest);
    mp_load_method(MP_OBJ_TO_PTR(args[0]), MP_QSTR_draw, self->draw_dest);

    ENGINE_INFO_PRINTF("Creating new Vector3 for BitmapSprite Node");
    self->position = vector3_class_new(&vector3_class_type, 0, 0, NULL);
    self->bitmap_data = NULL;
    self->width = 0;
    self->height = 0;
    self->frame_count = 0;
    self->animation_fps = 30.0f;
    self->rotation = 0.0f;
    self->scale = 100.0f;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t bitmap_sprite_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("BitmapSpriteNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_bitmap_sprite_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(bitmap_sprite_node_class_del_obj, bitmap_sprite_node_class_del);


STATIC mp_obj_t bitmap_sprite_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("BitmapSpriteNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(bitmap_sprite_node_class_tick_obj, bitmap_sprite_node_class_tick);


STATIC mp_obj_t bitmap_sprite_node_class_draw(mp_obj_t self_in, mp_obj_t camera_obj){
    ENGINE_INFO_PRINTF("BitmapSpriteNode: Drawing");

    engine_camera_node_class_obj_t *camera = MP_OBJ_TO_PTR(camera_obj);
    vector3_class_obj_t *camera_position = camera->position;
    
    camera_position->z += 0.01;
    camera_position->x = (20.0 * cos(camera_position->z));
    camera_position->y = (20.0 * sin(camera_position->z));

    // camera_position->x = camera_position->x + (mp_float_t)0.05f;
    // camera_position->y = camera_position->y + (mp_float_t)0.05f;

    engine_draw_pixel(0b1111100000000000, 128/4, 128/4, camera);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(bitmap_sprite_node_class_draw_obj, bitmap_sprite_node_class_draw);


STATIC mp_obj_t bitmap_sprite_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_bitmap_sprite_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);
    self->node_base.layer = (uint16_t)mp_obj_get_int(layer);
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(bitmap_sprite_node_class_set_layer_obj, bitmap_sprite_node_class_set_layer);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void bitmap_sprite_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    engine_bitmap_sprite_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_width:
                destination[0] = (mp_obj_t*)(&self->width);
            break;
            case MP_QSTR_height:
                destination[0] = (mp_obj_t*)(&self->height);
            break;
            case MP_QSTR_frame_count:
                destination[0] = (mp_obj_t*)(&self->frame_count);
            break;
            case MP_QSTR_animation_fps:
                destination[0] = (mp_obj_t*)(&self->animation_fps);
            break;
            case MP_QSTR_rotation:
                destination[0] = (mp_obj_t*)(&self->rotation);
            break;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                ENGINE_WARNING_PRINTF("Setting position not implemented!");
            break;
            case MP_QSTR_width:
                self->width = (mp_uint_t)destination[1];
            break;
            case MP_QSTR_height:
                self->height = (mp_uint_t)destination[1];
            break;
            case MP_QSTR_frame_count:
                self->frame_count = (mp_uint_t)destination[1];
            break;
            case MP_QSTR_animation_fps:
                self->animation_fps = (mp_uint_t)destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = (mp_uint_t)destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t bitmap_sprite_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),     MP_ROM_PTR(&bitmap_sprite_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick),        MP_ROM_PTR(&bitmap_sprite_node_class_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw),        MP_ROM_PTR(&bitmap_sprite_node_class_draw_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_layer),   MP_ROM_PTR(&bitmap_sprite_node_class_set_layer_obj) },
};


// Class init
STATIC MP_DEFINE_CONST_DICT(bitmap_sprite_node_class_locals_dict, bitmap_sprite_node_class_locals_dict_table);

const mp_obj_type_t engine_bitmap_sprite_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_BitmapSpriteNode,
    .print = bitmap_sprite_node_class_print,
    .make_new = bitmap_sprite_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = bitmap_sprite_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&bitmap_sprite_node_class_locals_dict,
};