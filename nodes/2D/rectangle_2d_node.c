#include "rectangle_2d_node.h"

#include "../node_types.h"
#include "nodes/camera_node.h"
#include "utility/debug_print.h"
#include "../../engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "display/engine_display_draw.h"

// Class required functions
STATIC void rectangle_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Rectangle2DNode");
}


STATIC mp_obj_t rectangle_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Rectangle2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_2d_node_class_tick_obj, rectangle_2d_node_class_tick);


STATIC mp_obj_t rectangle_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_obj){
    ENGINE_INFO_PRINTF("Rectangle2DNode: Drawing");

    engine_rectangle_2d_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    engine_camera_node_class_obj_t *camera = MP_OBJ_TO_PTR(camera_obj);

    // For whatever reason, position being a struct within self
    // means that we don't need look up the position attribute.
    // Only things like self.width need to be looked up
    vector2_class_obj_t *position = self->position;
    mp_int_t width = mp_obj_get_int(mp_load_attr(self->access, MP_QSTR_width));
    mp_int_t height = mp_obj_get_int(mp_load_attr(self->access, MP_QSTR_height));
    mp_int_t color = mp_obj_get_int(mp_load_attr(self->access, MP_QSTR_color));

    // Rotation not implemented yet so this is simple!
    for(mp_int_t y=0; y<height; y++){
        for(mp_int_t x=0; x<width; x++){
            engine_draw_pixel(color, (int32_t)position->x+x, (int32_t)position->y+y, camera);
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rectangle_2d_node_class_draw_obj, rectangle_2d_node_class_draw);


mp_obj_t rectangle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Rectangle2DNode");
    
    // Check that there's an argument that's hopefully a reference to the inheriting subclass
    mp_arg_check_num(n_args, n_kw, 0, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_rectangle_2d_node_class_obj_t *self = m_new_obj_with_finaliser(engine_rectangle_2d_node_class_obj_t);
    self->base.type = &engine_rectangle_2d_node_class_type;
    self->node_base.layer = 0;
    self->node_base.type = NODE_TYPE_RECTANGLE_2D;
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);
    node_base_set_if_visible(&self->node_base, true);
    node_base_set_if_disabled(&self->node_base, false);
    node_base_set_if_just_added(&self->node_base, true);

    // Handle setting callbacks depending on if this class was inherited or not
    if(n_args == 0){
        ENGINE_INFO_PRINTF("Rectangle2DNode: Does not have child class");
        self->access = self;
        self->tick_dest[0] = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);
        self->draw_dest[0] = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_draw_obj);
    }else{
        ENGINE_INFO_PRINTF("Rectangle2DNode: Does have child class");
        self->access = args[0];
        mp_load_method(self->access, MP_QSTR_tick, self->tick_dest);
        mp_load_method(self->access, MP_QSTR_draw, self->draw_dest);
    }


    ENGINE_INFO_PRINTF("Creating new Vector2 for BitmapSprite Node");
    self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    self->width = mp_obj_new_int(15);
    self->height = mp_obj_new_int(5);
    self->color = mp_obj_new_int(0xffff);

    // Set the self in the dest to the non inherited instance.
    // All getters/setters operate on the base class instance,
    // not the subclass
    self->tick_dest[1] = self;
    self->draw_dest[1] = self;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rectangle_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Rectangle2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_rectangle_2d_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_2d_node_class_del_obj, rectangle_2d_node_class_del);


STATIC mp_obj_t rectangle_2d_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_rectangle_2d_node_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);
    self->node_base.layer = (uint16_t)mp_obj_get_int(layer);
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rectangle_2d_node_class_set_layer_obj, rectangle_2d_node_class_set_layer);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void rectangle_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Rectangle2DNode attr");

    engine_rectangle_2d_node_class_obj_t *self = self_in;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_width:
                self->width = destination[1];
            break;
            case MP_QSTR_height:
                self->height = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_2d_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__),     MP_ROM_PTR(&rectangle_2d_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick),        MP_ROM_PTR(&rectangle_2d_node_class_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_draw),        MP_ROM_PTR(&rectangle_2d_node_class_draw_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_layer),   MP_ROM_PTR(&rectangle_2d_node_class_set_layer_obj) }
};


// Class init
STATIC MP_DEFINE_CONST_DICT(rectangle_2d_node_class_locals_dict, rectangle_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_rectangle_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Rectangle2DNode,
    .print = rectangle_2d_node_class_print,
    .make_new = rectangle_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = rectangle_2d_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&rectangle_2d_node_class_locals_dict,
};