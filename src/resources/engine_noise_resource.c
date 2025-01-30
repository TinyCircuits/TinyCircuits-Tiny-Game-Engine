#define FNL_IMPL

#include "engine_noise_resource.h"
#include "debug/debug_print.h"
#include <stdlib.h>

// Class required functions
static void noise_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): NoiseResource");
}


mp_obj_t noise_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New NoiseResource");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    noise_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(noise_resource_class_obj_t, &noise_resource_class_type);
    self->base.type = &noise_resource_class_type;

    self->fnl = fnlCreateState();
    
    return MP_OBJ_FROM_PTR(self);
}


/*  --- doc ---
    NAME: noise_3d
    ID: noise_3d
    DESC: Given three values usually representing a position, output one value of noise
    PARAM: [type=float]     [name=x]    [value=any]
    PARAM: [type=float]     [name=y]    [value=any]
    PARAM: [type=float]     [name=z]    [value=any]
    RETURN: float
*/
static mp_obj_t noise_resource_class_noise_3d(size_t n_args, const mp_obj_t *args){
    noise_resource_class_obj_t *self = args[0];

    float x = mp_obj_get_float(args[1]);
    float y = mp_obj_get_float(args[2]);
    float z = mp_obj_get_float(args[3]);

    return mp_obj_new_float(fnlGetNoise3D(&self->fnl, x, y, z));
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(noise_resource_class_noise_3d_obj, 4, 4, noise_resource_class_noise_3d);


/*  --- doc ---
    NAME: noise_2d
    ID: noise_2d
    DESC: Given two values usually representing a position, output one value of noise
    PARAM: [type=float]     [name=x]    [value=any]
    PARAM: [type=float]     [name=z]    [value=any]
    RETURN: float
*/
static mp_obj_t noise_resource_class_noise_2d(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t z_obj){
    noise_resource_class_obj_t *self = self_in;

    float x = mp_obj_get_float(x_obj);
    float z = mp_obj_get_float(z_obj);

    return mp_obj_new_float(fnlGetNoise2D(&self->fnl, x, z));
}
MP_DEFINE_CONST_FUN_OBJ_3(noise_resource_class_noise_2d_obj, noise_resource_class_noise_2d);


static mp_obj_t noise_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("NoiseResource: Deleted (freeing noise data)");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(noise_resource_class_del_obj, noise_resource_class_del);


/*  --- doc ---
    NAME: NoiseResource
    ID: NoiseResource
    DESC: Object for outputting various types of 2D and 3D noise data. Uses the FastNoiseLite library: https://github.com/Auburn/FastNoiseLite. Since most aspects of the FastNoiseLite library were just directly exposed, refer to the library for further documentation or try different configurations and see what happens, or try this web preview: https://auburn.github.io/FastNoiseLite/
    ATTR:   [type=function]         [name={ref_link:noise_3d}]                      [value=function]
    ATTR:   [type=function]         [name={ref_link:noise_2d}]                      [value=function]
    ATTR:   [type=int]              [name=seed]                                     [value=any (set this to different numbers for different variations of noise). Default: 13337]
    ATTR:   [type=float]            [name=frequency]                                [value=any (the frequency of all noise types, higher means more dense noise and lower means less dense). Default: 0.01]
    ATTR:   [type=int]              [name=noise_type]                               [value=NOISE_OPENSIMPLEX2 | NOISE_OPENSIMPLEX2S | NOISE_CELLULAR | NOISE_PERLIN | NOISE_VALUE_CUBIC | NOISE_VALUE (noise algorithm to use for 2D and 3D noise). Default: NOISE_OPENSIMPLEX2]
    ATTR:   [type=int]              [name=rotation_type_3d]                         [value=ROTATION_NONE | ROTATION_IMPROVE_XY_PLANE | ROTATION_IMPROVE_XZ_PLANES (3D noise rotation types). Default: ROTATION_NONE]
    ATTR:   [type=int]              [name=fractal_type]                             [value=FRACTAL_NONE | FRACTAL_FBM, FRACTAL_RIDGED | FRACTAL_PINGPONG | FRACTAL_DOMAIN_WARP_PROGRESSIVE | FRACTAL_DOMAIN_WARP_INDEPENDENT (algorithm for combining octaves for fractal noise types). Default: FRACTAL_NONE]
    ATTR:   [type=int]              [name=octaves]                                  [value=any (the more octaves the long the compute time and the more chaotic the noise gets). Default: 3]
    ATTR:   [type=float]            [name=lacunarity]                               [value=any (applies to fractal noise types, sort of like noise frequency). Default: 2.0]
    ATTR:   [type=float]            [name=gain]                                     [value=any (octave gain for all noise types). Default: 0.5]
    ATTR:   [type=float]            [name=weighted_strength]                        [value=any (octave weighting for none domain warp fractal noise types). Default: 0.0]
    ATTR:   [type=float]            [name=ping_pong_strength]                       [value=any (fractal ping pong effect strength). Default: 2.0]
    ATTR:   [type=int]              [name=cellular_distance_func]                   [value=CELLULAR_DISTANCE_EUCLIDEAN | CELLULAR_DISTANCE_EUCLIDEANSQ | CELLULAR_DISTANCE_MANHATTAN | CELLULAR_DISTANCE_HYBRID (Distance function for cellular noise). Default: CELLULAR_DISTANCE_EUCLIDEANSQ]
    ATTR:   [type=int]              [name=cellular_return_type]                     [value=CELLULAR_RETURN_TYPE_CELLVALUE | CELLULAR_RETURN_TYPE_DISTANCE | CELLULAR_RETURN_TYPE_DISTANCE2 | CELLULAR_RETURN_TYPE_DISTANCE2ADD | CELLULAR_RETURN_TYPE_DISTANCE2SUB | CELLULAR_RETURN_TYPE_DISTANCE2MUL | CELLULAR_RETURN_TYPE_DISTANCE2DIV (return type for cellular noise). Default: FNL_CELLULAR_RETURN_TYPE_DISTANCE]
    ATTR:   [type=float]            [name=cellular_jitter_mod]                      [value=any (Max distance cellular points are allowed to move from their grid positions). Default: 1.0]
    ATTR:   [type=int]              [name=domain_warp_type]                         [value=DOMAIN_WARP_OPENSIMPLEX2 | DOMAIN_WARP_OPENSIMPLEX2_REDUCED | DOMAIN_WARP_BASICGRID (algorithm for domain warp). Default: DOMAIN_WARP_OPENSIMPLEX2]
    ATTR:   [type=float]            [name=domain_warp_amp]                          [value=any (Max distance from original position while using domain warp). Default: 1.0]

    ATTR:   [type=int]              [name=NOISE_OPENSIMPLEX2]                       [value=0]
    ATTR:   [type=int]              [name=NOISE_OPENSIMPLEX2S]                      [value=1]
    ATTR:   [type=int]              [name=NOISE_CELLULAR]                           [value=2]
    ATTR:   [type=int]              [name=NOISE_PERLIN]                             [value=3]
    ATTR:   [type=int]              [name=NOISE_VALUE_CUBIC]                        [value=4]
    ATTR:   [type=int]              [name=NOISE_VALUE]                              [value=5]

    ATTR:   [type=int]              [name=ROTATION_NONE]                            [value=0]
    ATTR:   [type=int]              [name=ROTATION_IMPROVE_XY_PLANE]                [value=1]
    ATTR:   [type=int]              [name=ROTATION_IMPROVE_XZ_PLANES]               [value=2]

    ATTR:   [type=int]              [name=FRACTAL_NONE]                             [value=0]
    ATTR:   [type=int]              [name=FRACTAL_FBM]                              [value=1]
    ATTR:   [type=int]              [name=FRACTAL_RIDGED]                           [value=2]
    ATTR:   [type=int]              [name=FRACTAL_PINGPONG]                         [value=3]
    ATTR:   [type=int]              [name=FRACTAL_DOMAIN_WARP_PROGRESSIVE]          [value=4]
    ATTR:   [type=int]              [name=FRACTAL_FRACTAL_DOMAIN_WARP_INDEPENDENT]  [value=5]

    ATTR:   [type=int]              [name=CELLULAR_DISTANCE_EUCLIDEAN]              [value=0]
    ATTR:   [type=int]              [name=CELLULAR_DISTANCE_EUCLIDEANSQ]            [value=1]
    ATTR:   [type=int]              [name=CELLULAR_DISTANCE_MANHATTAN]              [value=2]
    ATTR:   [type=int]              [name=CELLULAR_DISTANCE_HYBRID]                 [value=3]

    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_CELLVALUE]           [value=0]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE]            [value=1]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE2]           [value=2]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE2ADD]        [value=3]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE2SUB]        [value=4]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE2MUL]        [value=5]
    ATTR:   [type=int]              [name=CELLULAR_RETURN_TYPE_DISTANCE2DIV]        [value=6]

    ATTR:   [type=int]              [name=DOMAIN_WARP_OPENSIMPLEX2]                 [value=0]
    ATTR:   [type=int]              [name=DOMAIN_WARP_OPENSIMPLEX2_REDUCED]         [value=1]
    ATTR:   [type=int]              [name=DOMAIN_WARP_BASICGRID]                    [value=2]
*/ 
static void noise_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
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
static const mp_rom_map_elem_t noise_resource_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_NOISE_OPENSIMPLEX2), MP_ROM_INT(FNL_NOISE_OPENSIMPLEX2) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_OPENSIMPLEX2S), MP_ROM_INT(FNL_NOISE_OPENSIMPLEX2S) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_CELLULAR), MP_ROM_INT(FNL_NOISE_CELLULAR) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_PERLIN), MP_ROM_INT(FNL_NOISE_PERLIN) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_VALUE_CUBIC), MP_ROM_INT(FNL_NOISE_VALUE_CUBIC) },
    { MP_ROM_QSTR(MP_QSTR_NOISE_VALUE), MP_ROM_INT(FNL_NOISE_VALUE) },

    { MP_ROM_QSTR(MP_QSTR_ROTATION_NONE), MP_ROM_INT(FNL_ROTATION_NONE) },
    { MP_ROM_QSTR(MP_QSTR_ROTATION_IMPROVE_XY_PLANES), MP_ROM_INT(FNL_ROTATION_IMPROVE_XY_PLANES) },
    { MP_ROM_QSTR(MP_QSTR_ROTATION_IMPROVE_XZ_PLANES), MP_ROM_INT(FNL_ROTATION_IMPROVE_XZ_PLANES) },

    { MP_ROM_QSTR(MP_QSTR_FRACTAL_NONE), MP_ROM_INT(FNL_FRACTAL_NONE) },
    { MP_ROM_QSTR(MP_QSTR_FRACTAL_FBM), MP_ROM_INT(FNL_FRACTAL_FBM) },
    { MP_ROM_QSTR(MP_QSTR_FRACTAL_RIDGED), MP_ROM_INT(FNL_FRACTAL_RIDGED) },
    { MP_ROM_QSTR(MP_QSTR_FRACTAL_PINGPONG), MP_ROM_INT(FNL_FRACTAL_PINGPONG) },
    { MP_ROM_QSTR(MP_QSTR_FRACTAL_DOMAIN_WARP_PROGRESSIVE), MP_ROM_INT(FNL_FRACTAL_DOMAIN_WARP_PROGRESSIVE) },
    { MP_ROM_QSTR(MP_QSTR_FRACTAL_DOMAIN_WARP_INDEPENDENT), MP_ROM_INT(FNL_FRACTAL_DOMAIN_WARP_INDEPENDENT) },

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
static MP_DEFINE_CONST_DICT(noise_resource_class_locals_dict, noise_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    noise_resource_class_type,
    MP_QSTR_NoiseResource,
    MP_TYPE_FLAG_NONE,

    make_new, noise_resource_class_new,
    print, noise_resource_class_print,
    attr, noise_resource_class_attr,
    locals_dict, &noise_resource_class_locals_dict
);