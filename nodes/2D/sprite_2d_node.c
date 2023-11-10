#include "sprite_2d_node.h"

#include "../node_types.h"
#include "utility/debug_print.h"
#include "../../engine_object_layers.h"
#include "math/vector2.h"
#include "display/engine_display_draw.h"

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


STATIC mp_obj_t sprite_2d_node_class_draw(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Sprite2DNode: Drawing");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(sprite_2d_node_class_draw_obj, sprite_2d_node_class_draw);


mp_obj_t sprite_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Sprite2DNode");
    
    engine_sprite_2d_node_common_data_t *common_data = malloc(sizeof(engine_sprite_2d_node_common_data_t));

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

    mp_obj_t default_scale_parameters[2];
    default_scale_parameters[0] = mp_obj_new_float(1.0f);
    default_scale_parameters[1] = mp_obj_new_float(1.0f);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_sprite_2d_node_class_obj_t *sprite_2d_node = m_malloc(sizeof(engine_sprite_2d_node_class_obj_t));
        node_base->node = sprite_2d_node;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&sprite_2d_node_class_draw_obj);

        sprite_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        sprite_2d_node->width = mp_obj_new_int(16);
        sprite_2d_node->height = mp_obj_new_int(16);
        sprite_2d_node->filename = mp_obj_new_str("", 0);
        sprite_2d_node->fps = mp_obj_new_int(5);
        sprite_2d_node->rotation = mp_obj_new_float(0.0f);
        sprite_2d_node->scale = vector2_class_new(&vector2_class_type, 2, 0, default_scale_parameters);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];

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

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_width, mp_obj_new_int(16));
        mp_store_attr(node_base->node, MP_QSTR_height, mp_obj_new_int(16));
        mp_store_attr(node_base->node, MP_QSTR_filename, mp_obj_new_str("", 0));
        mp_store_attr(node_base->node, MP_QSTR_fps, mp_obj_new_int(5));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_scale, vector2_class_new(&vector2_class_type, 2, 0, default_scale_parameters));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to Rectangle2DNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC mp_obj_t sprite_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Sprite2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    free(node_base->node_common_data);
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(sprite_2d_node_class_del_obj, sprite_2d_node_class_del);


STATIC mp_obj_t sprite_2d_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(sprite_2d_node_class_set_layer_obj, sprite_2d_node_class_set_layer);


STATIC mp_obj_t sprite_2d_node_class_get_layer(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Getting object layer...");

    engine_node_base_t *node_base = self_in;
    return mp_obj_new_int(node_base->layer);
}
MP_DEFINE_CONST_FUN_OBJ_1(sprite_2d_node_class_get_layer_obj, sprite_2d_node_class_get_layer);


STATIC void sprite_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Sprite2DNode attr");

    engine_sprite_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&sprite_2d_node_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&sprite_2d_node_class_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&sprite_2d_node_class_get_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
            break;
            case MP_QSTR_filename:
                destination[0] = self->filename;
            break;
            case MP_QSTR_fps:
                destination[0] = self->fps;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
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
            case MP_QSTR_filename:
                self->filename = destination[1];
            break;
            case MP_QSTR_fps:
                self->fps = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
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


// Class init
STATIC MP_DEFINE_CONST_DICT(sprite_2d_node_class_locals_dict, sprite_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_sprite_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Sprite2DNode,
    .print = sprite_2d_node_class_print,
    .make_new = sprite_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = sprite_2d_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&sprite_2d_node_class_locals_dict,
};