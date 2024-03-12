#include "rectangle_collision_shape_2d.h"
#include "math/engine_math.h"
#include <string.h>


// Class required functions
STATIC void rectangle_collision_shape_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): RectangleCollisionShape2D");
}


void rectangle_collision_shape_2d_recalculate(mp_obj_t self_in){
    rectangle_collision_shape_2d_class_obj_t *self = self_in;

    float half_width = mp_obj_get_float(self->width) * 0.5f;
    float half_height = mp_obj_get_float(self->height) * 0.5f;

    float x_traversal_cos = cosf(self->rotation) * half_width;
    float x_traversal_sin = sinf(self->rotation) * half_width;

    float y_traversal_cos = cosf(self->rotation + HALF_PI) * half_height;
    float y_traversal_sin = sinf(self->rotation + HALF_PI) * half_height;

    // top-left
    self->vertices_x[0] = -x_traversal_cos + y_traversal_cos;
    self->vertices_y[0] =  x_traversal_sin - y_traversal_sin;

    // top-right
    self->vertices_x[1] =  x_traversal_cos + y_traversal_cos;
    self->vertices_y[1] = -x_traversal_sin - y_traversal_sin;

    // bottom-right
    self->vertices_x[2] =  x_traversal_cos - y_traversal_cos;
    self->vertices_y[2] = -x_traversal_sin + y_traversal_sin;

    // bottom-left
    self->vertices_x[3] = -x_traversal_cos - y_traversal_cos;
    self->vertices_y[3] =  x_traversal_sin + y_traversal_sin;


    // Calculate a new list of normals (should be able to
    // know the size of the final normal list size, just use
    // append for now: TODO)
    for(uint32_t ivx=0; ivx<4; ivx++){
        uint16_t next_ivx = 0;

        if(ivx + 1 < 4){
            next_ivx = ivx + 1;
        }else{
            next_ivx = 0;
        }
        
        // 2D Cross product (perpendicular vector to the direction of the edge): FLIP: https://stackoverflow.com/a/1243676
        float face_normal_x = self->vertices_x[next_ivx] - self->vertices_x[ivx];
        float face_normal_y = self->vertices_y[next_ivx] - self->vertices_y[ivx];

        float face_normal_length_squared = (face_normal_x*face_normal_x) + (face_normal_y*face_normal_y);

        // Flip sign of y-axis of normal since actually reversed on the screen
        float face_normal_length = sqrt(face_normal_length_squared);
        face_normal_x =  face_normal_x / face_normal_length;
        face_normal_y = -face_normal_y / face_normal_length;

        self->normals_x[ivx] = face_normal_x;
        self->normals_y[ivx] = face_normal_y;
    }
}


/* --- doc ---
   NAME: RectangleCollisionShape2D
   DESC: A rectangle used to describe the shape of collision for a {ref_link:Physics2DNode}.
   PARAM:  [type=float] [name=width]    [value=any]
   PARAM:  [type=float] [name=height]   [value=any]
   PARAM:  [type=float] [name=rotation] [value=radians]
   ATTR:   [type=float] [name=width]    [value=any]
   ATTR:   [type=float] [name=height]   [value=any]
   ATTR:   [type=float] [name=rotation] [value=radians]
*/
mp_obj_t rectangle_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Empty RectangleCollisionShape2D");
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_height,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {width, height};
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    if(parsed_args[width].u_obj == MP_OBJ_NULL) parsed_args[width].u_obj = mp_obj_new_float(5.0f);
    if(parsed_args[height].u_obj == MP_OBJ_NULL) parsed_args[height].u_obj = mp_obj_new_float(5.0f);

    rectangle_collision_shape_2d_class_obj_t *self = m_new_obj(rectangle_collision_shape_2d_class_obj_t);
    self->base.type = &rectangle_collision_shape_2d_class_type;
    self->width = parsed_args[width].u_obj;
    self->height = parsed_args[height].u_obj;
    self->rotation = 0.0f;

    rectangle_collision_shape_2d_recalculate(self);

    return MP_OBJ_FROM_PTR(self);
}


STATIC void rectangle_collision_shape_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing RectangleCollisionShape2D attr");

    rectangle_collision_shape_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_width:
                self->width = destination[1];
                rectangle_collision_shape_2d_recalculate(self);
            break;
            case MP_QSTR_height:
                self->height = destination[1];
                rectangle_collision_shape_2d_recalculate(self);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_collision_shape_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(rectangle_collision_shape_2d_class_locals_dict, rectangle_collision_shape_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rectangle_collision_shape_2d_class_type,
    MP_QSTR_RectangleCollisionShape2D,
    MP_TYPE_FLAG_NONE,

    make_new, rectangle_collision_shape_2d_class_new,
    print, rectangle_collision_shape_2d_class_print,
    attr, rectangle_collision_shape_2d_class_attr,
    locals_dict, &rectangle_collision_shape_2d_class_locals_dict
);