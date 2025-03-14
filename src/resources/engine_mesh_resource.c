#include "engine_mesh_resource.h"
#include "debug/debug_print.h"
#include "draw/engine_color.h"
#include "resources/engine_resource_manager.h"
#include "draw/engine_color.h"
#include "math/engine_math.h"
#include "math/vector3.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Size of buffer used for parsing lines of .obj files
#define TEMP_PARSE_BUFFER_SIZE 512


typedef enum mesh_pack_type_enum {NOPACK=0, PACKFLOAT=1} mesh_pack_type;


void load_obj_get_counts(char *read_line_buffer, uint32_t *vertex_count, uint32_t *face_count){
    while(true){
        // Reset these for every line
        char c = ' ';
        uint8_t index = 0;

        // Read until end of line but check first characters for 'v' or 'f'
        // indicating if the line contains information on vertex or face
        while(true){
            // Read a character and track how many have been read (end if nothing read, EOF)
            if(engine_file_read(0, &c, 1) == 0){
                return;
            }
            read_line_buffer[index] = c;
            index++;

            // Stop reading line if at end of it
            if(c == '\r' || c == '\n'){
                break;
            }

            // If we have read two characters, see if face or vertex count can be increased
            if(index == 2){
                if(read_line_buffer[0] == 'v' && read_line_buffer[1] == ' '){
                    *vertex_count += 1;
                }else if(read_line_buffer[0] == 'f' && read_line_buffer[1] == ' '){
                    *face_count += 1;
                }
            }
        }
    }
}


void load_obj_file(mesh_resource_class_obj_t *self, mp_obj_str_t *obj_path_mp, int pack_type){
    // Buffer for reading each line of .obj into to parse
    char read_line_buffer[128];

    // Open the file
    engine_file_open_read(0, obj_path_mp);

    // #1: Figure out how many vertices and indices/uvs there are. The number of vertices
    // can be different from the number of indices/uvs. The number of vertices and uvs should
    // be the same
    uint32_t vertex_count = 0;
    uint32_t face_count = 0;
    load_obj_get_counts(read_line_buffer, &vertex_count, &face_count);

    ENGINE_PRINTF("%ld %ld", vertex_count, face_count);
    

    // Close the file
    engine_file_close(0);
}


mp_obj_t mesh_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New MeshResource");

    // Too many combinations
    if(n_kw > 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Cannot create `MeshResource` with keyword arguments, only use positional arguments!"));
    }


    mesh_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(mesh_resource_class_obj_t, &mesh_resource_class_type);
    self->base.type = &mesh_resource_class_type;

    // Constructor paramater combinations:
    //  1. nothing: .vertices, .indices, and .uvs are set to empty `lists`
    //  2. path, in_ram: .vertices, .indices, and .uvs are set to `bytearray`s depending on the data in the .obj file otherwise set to empty `list`
    //  3. vertices, indices, and uvs: .vertices, .indices, and .uvs are set to passed `lists` or `bytearrays`, otherwise, set to empty `list`
    //  4. vertices, indices, uvs and colors: .vertices, .indices, .uvs, and .triangle_colors, are set to passed `lists` or `bytearrays`, otherwise, set to empty `list`

    self->vertex_count = mp_const_none;

    if(n_args == 0){
        self->vertices = mp_obj_new_list(0, NULL);
        self->indices = mp_obj_new_list(0, NULL);
        self->uvs = mp_obj_new_list(0, NULL);
        self->triangle_colors = mp_obj_new_list(0, NULL);
    }else{
        if(n_args == 1){
            if(mp_obj_is_str(args[0])){                                                                            // path
                // Open .obj mesh in FLASH
                load_obj_file(self, args[0], NOPACK);
                // mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: using .obj files is not implemented yet!"));
            }else if(mp_obj_is_type(args[0], &mp_type_list) || mp_obj_is_type(args[0], &mp_type_bytearray)){       // vertices
                self->vertices = args[0];
                self->indices = mp_obj_new_list(0, NULL);
                self->uvs = mp_obj_new_list(0, NULL);
                self->triangle_colors = mp_obj_new_list(0, NULL);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Expected first argument to be a `str` or `list/bytearray`, got `%s`"), mp_obj_get_type_str(args[0]));
            }
        }else if(n_args == 2){
            if(mp_obj_is_str(args[0]) && mp_obj_is_bool(args[1])){                                                  // path, in_ram
                // Open .obj mesh in FLASH or RAM
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: using .obj files is not implemented yet!"));
            }else if((mp_obj_is_type(args[0], &mp_type_list) || mp_obj_is_type(args[0], &mp_type_bytearray)) &&
                     (mp_obj_is_type(args[1], &mp_type_list) || mp_obj_is_type(args[1], &mp_type_bytearray))){       // vertices, indices
                self->vertices = args[0];
                self->indices = args[1];
                self->uvs = mp_obj_new_list(0, NULL);
                self->triangle_colors = mp_obj_new_list(0, NULL);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Expected the two arguments to be `str` and `bool` or `list/bytearray` and `list/bytearray`, got `%s` and `%s`"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]));
            }
        }else if(n_args == 3){
            if((mp_obj_is_type(args[0], &mp_type_list) || mp_obj_is_type(args[0], &mp_type_bytearray)) &&
               (mp_obj_is_type(args[1], &mp_type_list) || mp_obj_is_type(args[1], &mp_type_bytearray)) &&
               (mp_obj_is_type(args[2], &mp_type_list) || mp_obj_is_type(args[2], &mp_type_bytearray))){             // vertices, indices, uvs
                self->vertices = args[0];
                self->indices = args[1];
                self->uvs = args[2];
                self->triangle_colors = mp_obj_new_list(0, NULL);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Expected the three arguments to be `list/bytearray`, `list/bytearray`, and `list/bytearray`, got `%s`, `%s`, and `%s`"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]), mp_obj_get_type_str(args[2]));
            }
        }else if(n_args == 4){
            if((mp_obj_is_type(args[0], &mp_type_list) || mp_obj_is_type(args[0], &mp_type_bytearray)) &&
               (mp_obj_is_type(args[1], &mp_type_list) || mp_obj_is_type(args[1], &mp_type_bytearray)) &&
               (mp_obj_is_type(args[2], &mp_type_list) || mp_obj_is_type(args[2], &mp_type_bytearray)) &&
               (mp_obj_is_type(args[3], &mp_type_list) || mp_obj_is_type(args[3], &mp_type_bytearray))){             // vertices, indices, uvs, triangle_colors
                self->vertices = args[0];
                self->indices = args[1];
                self->uvs = args[2];
                self->triangle_colors = args[3];
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Expected the four arguments to be `list/bytearray`, `list/bytearray`, `list/bytearray`, and `list/bytearray`, got `%s`, `%s`, `%s`, and `%s`"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]), mp_obj_get_type_str(args[2]), mp_obj_get_type_str(args[3]));
            }
        }else{
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("MeshResource: ERROR: Too many arguments! Expected at most `4`, got `%d`"), n_args);
        }
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
static mp_obj_t mesh_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("MeshResource: Deleted");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mesh_resource_class_del_obj, mesh_resource_class_del);


/*  --- doc ---
    NAME: MeshResource
    ID: MeshResource
    DESC: Holds vertex and UV information.
*/ 
static void mesh_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing MeshResource attr");

    mesh_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&mesh_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_vertices:
                destination[0] = self->vertices;
            break;
            case MP_QSTR_indices:
                destination[0] = self->indices;
            break;
            case MP_QSTR_uvs:
                destination[0] = self->uvs;
            break;
            case MP_QSTR_triangle_colors:
                destination[0] = self->triangle_colors;
            break;
            case MP_QSTR_vertex_count:
                destination[0] = self->vertex_count;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_vertices:
                self->vertices = destination[1];
            break;
            case MP_QSTR_indices:
                self->indices = destination[1];
            break;
            case MP_QSTR_uvs:
                self->uvs = destination[1];
            break;
            case MP_QSTR_triangle_colors:
                self->triangle_colors = destination[1];
            break;
            case MP_QSTR_vertex_count:
                self->vertex_count = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t mesh_resource_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_PACKFLOAT), MP_ROM_INT(0) },
};
static MP_DEFINE_CONST_DICT(mesh_resource_class_locals_dict, mesh_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    mesh_resource_class_type,
    MP_QSTR_MeshResource,
    MP_TYPE_FLAG_NONE,

    make_new, mesh_resource_class_new,
    attr, mesh_resource_class_attr,
    locals_dict, &mesh_resource_class_locals_dict
);