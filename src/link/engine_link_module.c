#include "engine_link_module.h"
#include "py/obj.h"


#if defined(__EMSCRIPTEN__)

#elif defined(__unix__)

#elif defined(__arm__)
    #include "engine_link_rp3.h"
#endif


void engine_link_module_task(){
    engine_link_task();
}


static mp_obj_t engine_link_module_connected(){
    if(engine_link_connected()){
        return mp_const_true;
    }else{
        return mp_const_false;
    }
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_connected_obj, engine_link_module_connected);


static mp_obj_t engine_link_module_start(){
    engine_link_start();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_start_obj, engine_link_module_start);


static mp_obj_t engine_link_module_stop(){
    engine_link_stop();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_stop_obj, engine_link_module_stop);


/*  --- doc ---
    NAME: engine_link
    ID: engine_link
    DESC: Module sending data from one Thumby Color to another
*/ 
static const mp_rom_map_elem_t engine_link_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_link) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connected), (mp_obj_t)&engine_link_module_connected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_link_module_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&engine_link_module_stop_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_link_globals, engine_link_globals_table);

const mp_obj_module_t engine_link_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_link_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_link, engine_link_user_cmodule);