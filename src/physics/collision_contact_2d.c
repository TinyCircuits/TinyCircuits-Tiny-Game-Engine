#include "collision_contact_2d.h"


mp_obj_t collision_contact_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New CollisionContact2D");
    mp_arg_check_num(n_args, n_kw, 0, 6, false);

    collision_contact_2d_class_obj_t *self = m_new_obj(collision_contact_2d_class_obj_t);
    self->base.type = &collision_contact_2d_class_type;

    self->penetration.base.type = &mp_type_float;

    if(n_args == 0){
        self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        self->normal = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    }else if(n_args == 6){
        mp_obj_t parameters[2];
        parameters[0] = args[0];
        parameters[1] = args[1];
        self->position = vector2_class_new(&vector2_class_type, 2, 0, parameters);

        parameters[0] = args[2];
        parameters[1] = args[3];
        self->normal = vector2_class_new(&vector2_class_type, 2, 0, parameters);
        
        self->penetration.value = mp_obj_get_float(args[4]);

        self->node = args[5];
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("CollisionContact2D: Expected 0 or 4 arguments, got something else..."));
    }
    
    return MP_OBJ_FROM_PTR(self);
}


/* --- doc ---
   NAME: CollisionContact2D
   ID: CollisionContact2D
   DESC: Object that contains information about a collision
   ATTR: [type={ref_link:Vector2}] [name=position]      [value={ref_link:Vector2} TODO: implement filling this out upon collision of polygons, not easy...]
   ATTR: [type={ref_link:Vector2}] [name=normal]        [value={ref_link:Vector2}]
   ATTR: [type=float]              [name=penetration]   [value=any]
   ATTR: [type=object]             [name=node]          [value=object (the other node in the collision)]
*/ 
static void collision_contact_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CollisionContact2D attr");

    collision_contact_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_normal:
                destination[0] = self->normal;
            break;
            case MP_QSTR_penetration:
                destination[0] = mp_obj_new_float(self->penetration.value);
            break;
            case MP_QSTR_node:
                destination[0] = self->node;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_normal:
                self->normal = destination[1];
            break;
            case MP_QSTR_penetration:
                self->penetration.value = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_node:
                self->node = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t collision_contact_2d_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(collision_contact_2d_class_locals_dict, collision_contact_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    collision_contact_2d_class_type,
    MP_QSTR_CollisionContact2D,
    MP_TYPE_FLAG_NONE,

    make_new, collision_contact_2d_class_new,
    attr, collision_contact_2d_class_attr,
    locals_dict, &collision_contact_2d_class_locals_dict
);