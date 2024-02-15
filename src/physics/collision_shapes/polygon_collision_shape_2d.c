#include "polygon_collision_shape_2d.h"
#include "math/engine_math.h"
#include <string.h>


// Class required functions
STATIC void polygon_collision_shape_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): PolygonCollisionShape2D");
}


/* --- doc ---
   NAME: calculate_normals
   DESC: Recalculates normals for the shape. Automaticaly called when instance of this is created and when list of vertices is directly changed. Does not get automaticaly called when appending/inserting/deleting points from 'vertices'
   RETURN: None
*/
STATIC mp_obj_t polygon_2d_node_class_calculate_normals(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Polygon2DNode: Calculating normals");

    polygon_collision_shape_2d_class_obj_t *self = self_in;

    mp_obj_list_t *normals = self->normals;
    mp_obj_list_t *vertices = self->vertices;

    // Clear the list of normals
    normals->len = 0;
    normals->items = m_renew(mp_obj_t, normals->items, normals->alloc, 4);
    normals->alloc = 4;
    mp_seq_clear(normals->items, 0, normals->alloc, sizeof(*normals->items));

    // Calculate a new list of normals (should be able to
    // know the size of the final normal list size, just use
    // append for now: TODO)
    for(uint32_t ivx=0; ivx<vertices->len; ivx++){
        uint16_t next_ivx = 0;

        if(ivx + 1 < vertices->len){
            next_ivx = ivx + 1;
        }else{
            next_ivx = 0;
        }

        float temp_face_normal_x = ((vector2_class_obj_t*)vertices->items[next_ivx])->x - ((vector2_class_obj_t*)vertices->items[ivx])->x;
        float temp_face_normal_y = ((vector2_class_obj_t*)vertices->items[next_ivx])->y - ((vector2_class_obj_t*)vertices->items[ivx])->y;
        
        // 2D Cross product (perpendicular vector to the direction of the edge)
        float face_normal_x = -temp_face_normal_y;
        float face_normal_y = temp_face_normal_x;

        float face_normal_length_squared = (face_normal_x*face_normal_x) + (face_normal_y*face_normal_y);

        if(face_normal_length_squared < EPSILON * EPSILON){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("PolygonCollisionShape2D: Could not calculate polygon collision normals, zero length edge detected"));
        }

        float face_normal_length = sqrt(face_normal_length_squared);
        face_normal_x = face_normal_x / face_normal_length;
        face_normal_y = face_normal_y / face_normal_length;

        ENGINE_INFO_PRINTF("PolygonCollisionShape2D: Calculated face normal: %.03f %.03f", face_normal_x, face_normal_y);

        // Could avoid a bunch of reallocations if calculate number of resulting normals: TODO
        mp_obj_list_append(self->normals, vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(face_normal_x), mp_obj_new_float(face_normal_y)}));
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(polygon_2d_node_class_calculate_normals_obj, polygon_2d_node_class_calculate_normals);


/* --- doc ---
   NAME: PolygonCollisionShape2D
   DESC: A polygon used to describe the shape of collision for a {ref_link:Physics2DNode}.
   PARAM:  [type=list of {ref_link:Vector2}]         [name=vertices]                         [value=list of {ref_link:Vector2}]
   PARAM:  [type=list of {ref_link:FontResource}]    [name=normals]                          [value=list of {ref_link:FontResource}]
   ATTR:   [type=function]                           [name={ref_link:calculate_normals}]     [value=function]
   ATTR:   [type=list]                               [name=vertices]                         [value=list of {ref_link:Vector2}s]
   ATTR:   [type=list]                               [name=normals]                          [value=list of {ref_link:Vector2}s]
*/
mp_obj_t poly_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Empty PolygonCollisionShape2D");
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_vertices,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_normals,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {vertices, normals};
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    if(parsed_args[vertices].u_obj == MP_OBJ_NULL) parsed_args[vertices].u_obj = mp_obj_new_list(0, NULL);
    if(parsed_args[normals].u_obj == MP_OBJ_NULL) parsed_args[normals].u_obj = mp_obj_new_list(0, NULL);

    polygon_collision_shape_2d_class_obj_t *self = m_new_obj(polygon_collision_shape_2d_class_obj_t);
    self->base.type = &polygon_collision_shape_2d_class_type;
    self->vertices = parsed_args[vertices].u_obj;
    self->normals = parsed_args[normals].u_obj;

    polygon_2d_node_class_calculate_normals(self);

    return MP_OBJ_FROM_PTR(self);
}


// /* 
//    NAME: EmptyPolyCollisionShape2D
//    DESC: Dissolves into a {ref_link:PolygonCollisionShape2D} object that contains no vertices or normals (this is the default for {ref_link:PolygonCollisionShape2D})
// */
// mp_obj_t empty_poly_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
//     ENGINE_INFO_PRINTF("New Empty PolygonCollisionShape2D");
//     mp_arg_check_num(n_args, n_kw, 0, 0, false);

//     polygon_collision_shape_2d_class_obj_t *self = m_new_obj(polygon_collision_shape_2d_class_obj_t);
//     self->base.type = &polygon_collision_shape_2d_class_type;
//     self->vertices = mp_obj_new_list(0, NULL);
//     self->normals = mp_obj_new_list(0, NULL);
    
//     return MP_OBJ_FROM_PTR(self);
// }


// /* 
//    NAME: RectanglePolyCollisionShape2D
//    DESC: A basic polygon used to describe the shape of collision for a {ref_link:Physics2DNode}. This constructor dissolves into a {ref_link:PolygonCollisionShape2D} object but instead of being empty, it is filled with vertices and normals for a rectangle
// */
// mp_obj_t rectangle_poly_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
//     ENGINE_INFO_PRINTF("New Rectangle PolygonCollisionShape2D");
//     mp_arg_check_num(n_args, n_kw, 0, 2, false);

//     polygon_collision_shape_2d_class_obj_t *self = m_new_obj(polygon_collision_shape_2d_class_obj_t);
//     self->base.type = &polygon_collision_shape_2d_class_type;

//     if(n_args == 0){
//         self->vertices = mp_obj_new_list(4, (mp_obj_t[]){vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f), mp_obj_new_float(1.0f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f),  mp_obj_new_float(1.0f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f),  mp_obj_new_float(-1.0f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f), mp_obj_new_float(-1.0f)})});
//     }else if(n_args == 1){
//         float scale = mp_obj_get_float(args[0]);
//         self->vertices = mp_obj_new_list(4, (mp_obj_t[]){vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f*scale), mp_obj_new_float(1.0f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f*scale),  mp_obj_new_float(1.0f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f*scale),  mp_obj_new_float(-1.0f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-1.0f*scale), mp_obj_new_float(-1.0f*scale)})});
//     }else if(n_args == 2){
//         float width = mp_obj_get_float(args[0]);
//         float height = mp_obj_get_float(args[1]);
//         self->vertices = mp_obj_new_list(4, (mp_obj_t[]){vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-width/2), mp_obj_new_float(height/2)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(width/2),  mp_obj_new_float(height/2)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(width/2),  mp_obj_new_float(-height/2)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-width/2), mp_obj_new_float(-height/2)})});
//     }
//     self->normals = mp_obj_new_list(0, NULL);
    
//     polygon_2d_node_class_calculate_normals(self);

//     return MP_OBJ_FROM_PTR(self);
// }


// /* 
//    NAME: HexagonPolyCollisionShape2D
//    DESC: A basic polygon used to describe the shape of collision for a {ref_link:Physics2DNode}. This constructor dissolves into a {ref_link:PolygonCollisionShape2D} object but instead of being empty, it is filled with vertices and normals for a hexagon
// */
// mp_obj_t hexagon_poly_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
//     ENGINE_INFO_PRINTF("New Hexagon PolygonCollisionShape2D");
//     mp_arg_check_num(n_args, n_kw, 0, 1, false);

//     polygon_collision_shape_2d_class_obj_t *self = m_new_obj(polygon_collision_shape_2d_class_obj_t);
//     self->base.type = &polygon_collision_shape_2d_class_type;
//     if(n_args == 0){
//         self->vertices = mp_obj_new_list(6, (mp_obj_t[]){vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f),        mp_obj_new_float(1.0f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f),   mp_obj_new_float(0.5f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f),   mp_obj_new_float(-0.5f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f),        mp_obj_new_float(-1.0f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f),  mp_obj_new_float(-0.5f)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f),  mp_obj_new_float(0.5f)})});
//     }else if(n_args == 1){
//         float scale = mp_obj_get_float(args[0]);
//         self->vertices = mp_obj_new_list(6, (mp_obj_t[]){vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f*scale),        mp_obj_new_float(1.0f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f*scale),   mp_obj_new_float(0.5f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.866025f*scale),   mp_obj_new_float(-0.5f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f*scale),        mp_obj_new_float(-1.0f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f*scale),  mp_obj_new_float(-0.5f*scale)}),
//                                                         vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(-0.866025f*scale),  mp_obj_new_float(0.5f*scale)})});
//     }
//     self->normals = mp_obj_new_list(0, NULL);
    
//     polygon_2d_node_class_calculate_normals(self);

//     return MP_OBJ_FROM_PTR(self);
// }


STATIC void polygon_collision_shape_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PolygonCollisionShape2D attr");

    polygon_collision_shape_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_calculate_normals:
                destination[0] = MP_OBJ_FROM_PTR(&polygon_2d_node_class_calculate_normals_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_vertices:
                destination[0] = self->vertices;
            break;
            case MP_QSTR_normals:
                destination[0] = self->normals;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_vertices:
                self->vertices = destination[1];
                polygon_2d_node_class_calculate_normals(self);
            break;
            case MP_QSTR_normals:
                self->normals = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t polygon_collision_shape_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(polygon_collision_shape_2d_class_locals_dict, polygon_collision_shape_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    polygon_collision_shape_2d_class_type,
    MP_QSTR_PolygonCollisionShape2D,
    MP_TYPE_FLAG_NONE,

    make_new, poly_collision_shape_2d_class_new,
    print, polygon_collision_shape_2d_class_print,
    attr, polygon_collision_shape_2d_class_attr,
    locals_dict, &polygon_collision_shape_2d_class_locals_dict
);


// MP_DEFINE_CONST_OBJ_TYPE(
//     empty_poly_collision_shape_2d_class_type,
//     MP_QSTR_EmptyPolyCollisionShape2D,
//     MP_TYPE_FLAG_NONE,

//     make_new, empty_poly_collision_shape_2d_class_new,
//     print, polygon_collision_shape_2d_class_print,
//     attr, polygon_collision_shape_2d_class_attr,
//     locals_dict, &polygon_collision_shape_2d_class_locals_dict
// );


// MP_DEFINE_CONST_OBJ_TYPE(
//     rectangle_poly_collision_shape_2d_class_type,
//     MP_QSTR_RectanglePolyCollisionShape2D,
//     MP_TYPE_FLAG_NONE,

//     make_new, rectangle_poly_collision_shape_2d_class_new,
//     print, polygon_collision_shape_2d_class_print,
//     attr, polygon_collision_shape_2d_class_attr,
//     locals_dict, &polygon_collision_shape_2d_class_locals_dict
// );


// MP_DEFINE_CONST_OBJ_TYPE(
//     hexagon_poly_collision_shape_2d_class_type,
//     MP_QSTR_HexagonPolyCollisionShape2D,
//     MP_TYPE_FLAG_NONE,

//     make_new, hexagon_poly_collision_shape_2d_class_new,
//     print, polygon_collision_shape_2d_class_print,
//     attr, polygon_collision_shape_2d_class_attr,
//     locals_dict, &polygon_collision_shape_2d_class_locals_dict
// );