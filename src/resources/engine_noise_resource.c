#define FNL_IMPL

#include "engine_noise_resource.h"
#include "debug/debug_print.h"
#include <stdlib.h>

// Class required functions
STATIC void noise_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): NoiseResource");
}


mp_obj_t noise_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New NoiseResource");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    noise_resource_class_obj_t *self = m_new_obj_with_finaliser(noise_resource_class_obj_t);
    self->base.type = &noise_resource_class_type;

    self->fnl = fnlCreateState();
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t noise_resource_class_noise_3d(size_t n_args, const mp_obj_t *args){
    noise_resource_class_obj_t *self = args[0];

    float x = mp_obj_get_float(args[1]);
    float y = mp_obj_get_float(args[2]);
    float z = mp_obj_get_float(args[3]);

    return mp_obj_new_float(fnlGetNoise3D(&self->fnl, x, y, z));
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(noise_resource_class_noise_3d_obj, 4, 4, noise_resource_class_noise_3d);


STATIC mp_obj_t noise_resource_class_noise_2d(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t z_obj){
    noise_resource_class_obj_t *self = self_in;

    float x = mp_obj_get_float(x_obj);
    float z = mp_obj_get_float(z_obj);

    return mp_obj_new_float(fnlGetNoise2D(&self->fnl, x, z));
}
MP_DEFINE_CONST_FUN_OBJ_3(noise_resource_class_noise_2d_obj, noise_resource_class_noise_2d);


STATIC mp_obj_t noise_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("NoiseResource: Deleted (freeing noise data)");

    noise_resource_class_obj_t *self = self_in;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(noise_resource_class_del_obj, noise_resource_class_del);


STATIC void noise_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing NoiseResource attr");

    noise_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&noise_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_noise_3d:
                destination[0] = MP_OBJ_FROM_PTR(&noise_resource_class_noise_3d_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_noise_2d:
                destination[0] = MP_OBJ_FROM_PTR(&noise_resource_class_noise_2d_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_seed:
                destination[0] = mp_obj_new_int(self->fnl.seed);
            break;
            case MP_QSTR_frequency:
                destination[0] = mp_obj_new_float(self->fnl.frequency);
            break;
            case MP_QSTR_noise_type:
                destination[0] = mp_obj_new_int(self->fnl.noise_type);
            break;
            case MP_QSTR_rotation_type_3d:
                destination[0] = mp_obj_new_int(self->fnl.rotation_type_3d);
            break;
            case MP_QSTR_fractal_type:
                destination[0] = mp_obj_new_int(self->fnl.fractal_type);
            break;
            case MP_QSTR_octaves:
                destination[0] = mp_obj_new_int(self->fnl.octaves);
            break;
            case MP_QSTR_lacunarity:
                destination[0] = mp_obj_new_float(self->fnl.lacunarity);
            break;
            case MP_QSTR_gain:
                destination[0] = mp_obj_new_float(self->fnl.gain);
            break;
            case MP_QSTR_weighted_strength:
                destination[0] = mp_obj_new_float(self->fnl.weighted_strength);
            break;
            case MP_QSTR_ping_pong_strength:
                destination[0] = mp_obj_new_float(self->fnl.ping_pong_strength);
            break;
            case MP_QSTR_cellular_distance_func:
                destination[0] = mp_obj_new_int(self->fnl.cellular_distance_func);
            break;
            case MP_QSTR_cellular_return_type:
                destination[0] = mp_obj_new_int(self->fnl.cellular_return_type);
            break;
            case MP_QSTR_cellular_jitter_mod:
                destination[0] = mp_obj_new_float(self->fnl.cellular_jitter_mod);
            break;
            case MP_QSTR_domain_warp_amp:
                destination[0] = mp_obj_new_float(self->fnl.domain_warp_amp);
            break;
            case MP_QSTR_domain_warp_type:
                destination[0] = mp_obj_new_int(self->fnl.domain_warp_type);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_seed:
                self->fnl.seed = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_frequency:
                self->fnl.frequency = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_noise_type:
                self->fnl.noise_type = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_rotation_type_3d:
                self->fnl.rotation_type_3d = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_fractal_type:
                self->fnl.fractal_type = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_octaves:
                self->fnl.octaves = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_lacunarity:
                self->fnl.lacunarity = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_gain:
                self->fnl.gain = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_weighted_strength:
                self->fnl.weighted_strength = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_ping_pong_strength:
                self->fnl.ping_pong_strength = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_cellular_distance_func:
                self->fnl.cellular_distance_func = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_cellular_return_type:
                self->fnl.cellular_return_type = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_cellular_jitter_mod:
                self->fnl.cellular_jitter_mod = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_domain_warp_amp:
                self->fnl.domain_warp_amp = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_domain_warp_type:
                self->fnl.domain_warp_type = mp_obj_get_int(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t noise_resource_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_NOISE_OPENSIMPLEX2), MP_ROM_INT(FNL_NOISE_OPENSIMPLEX2) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_OPENSIMPLEX2S), MP_ROM_INT(FNL_NOISE_OPENSIMPLEX2S) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_CELLULAR), MP_ROM_INT(FNL_NOISE_CELLULAR) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_PERLIN), MP_ROM_INT(FNL_NOISE_PERLIN) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_VALUE_CUBIC), MP_ROM_INT(FNL_NOISE_VALUE_CUBIC) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_VALUE), MP_ROM_INT(FNL_NOISE_VALUE) },

    { MP_ROM_QSTR(MP_QSTR_ROTATION_NONE), MP_ROM_INT(FNL_ROTATION_NONE) },
    { MP_ROM_QSTR(MP_QSTR_ROTATION_IMPROVE_XY_PLANES), MP_ROM_INT(FNL_ROTATION_IMPROVE_XY_PLANES) },
    { MP_ROM_QSTR(MP_QSTR_ROTATION_IMPROVE_XZ_PLANES), MP_ROM_INT(FNL_ROTATION_IMPROVE_XZ_PLANES) },

    { MP_ROM_QSTR(MP_QSTR_CELLULAR_DISTANCE_EUCLIDEAN), MP_ROM_INT(FNL_CELLULAR_DISTANCE_EUCLIDEAN) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_DISTANCE_EUCLIDEANSQ), MP_ROM_INT(FNL_CELLULAR_DISTANCE_EUCLIDEANSQ) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_DISTANCE_MANHATTAN), MP_ROM_INT(FNL_CELLULAR_DISTANCE_MANHATTAN) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_DISTANCE_HYBRID), MP_ROM_INT(FNL_CELLULAR_DISTANCE_HYBRID) },

    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_CELLVALUE), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_CELLVALUE) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE2), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE2) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE2ADD), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE2ADD) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE2SUB), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE2SUB) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE2MUL), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE2MUL) },
    { MP_ROM_QSTR(MP_QSTR_CELLULAR_RETURN_TYPE_DISTANCE2DIV), MP_ROM_INT(FNL_CELLULAR_RETURN_TYPE_DISTANCE2DIV) },

    { MP_ROM_QSTR(MP_QSTR_DOMAIN_WARP_OPENSIMPLEX2), MP_ROM_INT(FNL_DOMAIN_WARP_OPENSIMPLEX2) },
    { MP_ROM_QSTR(MP_QSTR_DOMAIN_WARP_OPENSIMPLEX2_REDUCED), MP_ROM_INT(FNL_DOMAIN_WARP_OPENSIMPLEX2_REDUCED) },
    { MP_ROM_QSTR(MP_QSTR_DOMAIN_WARP_BASICGRID), MP_ROM_INT(FNL_DOMAIN_WARP_BASICGRID) },
};
STATIC MP_DEFINE_CONST_DICT(noise_resource_class_locals_dict, noise_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    noise_resource_class_type,
    MP_QSTR_NoiseResource,
    MP_TYPE_FLAG_NONE,

    make_new, noise_resource_class_new,
    print, noise_resource_class_print,
    attr, noise_resource_class_attr,
    locals_dict, &noise_resource_class_locals_dict
);