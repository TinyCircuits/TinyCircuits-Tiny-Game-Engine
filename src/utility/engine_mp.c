#include "engine_mp.h"


mp_obj_t engine_mp_load_attr_maybe(mp_obj_t base, qstr attr){
    mp_obj_t dest[2];
    mp_load_method_maybe(base, attr, dest);

    if(dest[0] == MP_OBJ_NULL){
        return MP_OBJ_NULL;
    }if(dest[1] == MP_OBJ_NULL){
        // load_method returned just a normal attribute
        return dest[0];
    }else{
        // load_method returned a method, so build a bound method object
        return mp_obj_new_bound_meth(dest[0], dest[1]);
    }
}