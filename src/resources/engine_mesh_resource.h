#ifndef ENGINE_MESH_RESOURCE_H
#define ENGINE_MESH_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_file.h"

typedef struct mesh_resource_class_obj_t{
    mp_obj_base_t base;
    mp_obj_t vertices;  // list of Vector3s or bytearray of triplet 4 byte float groups
    mp_obj_t indices;   // list of ints or bytearray of 4 byte ints
    mp_obj_t uvs;       // list of floats or bytearray of 4 byte floats
}mesh_resource_class_obj_t;

extern const mp_obj_type_t mesh_resource_class_type;
mp_obj_t mesh_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_MESH_RESOURCE_H