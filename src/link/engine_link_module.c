#include "engine_link_module.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

/* 
    The link between Thumby Colors happens by making one
    unit a USB `device` and the other a USB `host`.
    
    After calling `start()`, each unit will flip between
    being a `device` and checking if it is connected to a
    `host`, or being a `host` and waiting for the tudb mount
    cb to be invoked. This way, if `start()` is called when
    both units are not connected, they will have a chance to
    connect after being plugged into each other.

    After calling `start()`, a corresponding `stop()` will need
    to be called to make the unit into a USB `device` again.
    The unit will also be made into a USB device if it restarts.
*/

#if defined(__EMSCRIPTEN__)

#elif defined(__unix__)

#elif defined(__arm__)
    #include "engine_link_rp3.h"
#endif


bool was_connected = false;
mp_obj_t link_connected_cb = mp_const_none;
mp_obj_t link_disconnected_cb = mp_const_none;


// This needs to be called often, runs USB discovery and host-task
void engine_link_module_task(){
    engine_link_task();

    // As long as this method is being called, invoke callbacks
    // if connected or disconnected (tusb mount cbs can be weird)
    bool connected = engine_link_connected();

    if(link_connected_cb != mp_const_none && was_connected == true && connected == false){           // Was connected now we aren't, disconnected
        mp_call_function_0(link_disconnected_cb);
    }else if(link_disconnected_cb != mp_const_none && was_connected == false && connected == true){  // Was disconnected now we aren't, connected
        mp_call_function_0(link_connected_cb);
    }

    was_connected = connected;
}


// Reset callbacks and go back to being a USB device
void engine_link_module_reset(){
    link_connected_cb = mp_const_none;
    link_disconnected_cb = mp_const_none;
    engine_link_stop();
}


/*  --- doc ---
    NAME: connected
    ID: engine_link_connected
    DESC: Returns True if the Thumby Color is connected to another Thumby Color using a link cable
    RETURN: True or False
*/ 
static mp_obj_t engine_link_module_connected(){
    if(engine_link_connected()){
        return mp_const_true;
    }else{
        return mp_const_false;
    }
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_connected_obj, engine_link_module_connected);


/*  --- doc ---
    NAME: start
    ID: engine_link_start
    DESC: Starts link communication and starts discovery process.
    RETURN: None
*/ 
static mp_obj_t engine_link_module_start(){
    engine_link_start();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_start_obj, engine_link_module_start);


/*  --- doc ---
    NAME: stop
    ID: engine_link_stop
    DESC: Stops link communication and makes the Thumby a normal USB device again.
    RETURN: None
*/ 
static mp_obj_t engine_link_module_stop(){
    engine_link_stop();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_stop_obj, engine_link_module_stop);


/*  --- doc ---
    NAME: set_connected_cb
    ID: engine_link_set_connected_cb
    DESC: Provided a function, this function will be called when the Thumby Color connects to something
    RETURN: None
*/ 
static mp_obj_t engine_link_module_set_connected_cb(mp_obj_t connected_cb){
    link_connected_cb = connected_cb;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_link_module_set_connected_cb_obj, engine_link_module_set_connected_cb);


/*  --- doc ---
    NAME: set_disconnected_cb
    ID: engine_link_set_disconnected_cb
    DESC: Provided a function, this function will be called when the Thumby Color disconnects from something
    RETURN: None
*/ 
static mp_obj_t engine_link_module_set_disconnected_cb(mp_obj_t disconnected_cb){
    link_disconnected_cb = disconnected_cb;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_link_module_set_disconnected_cb_obj, engine_link_module_set_disconnected_cb);


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
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_connected_cb), (mp_obj_t)&engine_link_module_set_connected_cb_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_disconnected_cb), (mp_obj_t)&engine_link_module_set_disconnected_cb_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_link_globals, engine_link_globals_table);

const mp_obj_module_t engine_link_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_link_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_link, engine_link_user_cmodule);