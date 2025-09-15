#include "engine_link_module.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/objarray.h"
#include "py/binary.h"
#include "math/engine_math.h"

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
    #include "engine_link_unix.h"
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

    if(was_connected == true && connected == false){     // Was connected now we aren't, disconnected
        if(link_disconnected_cb != mp_const_none) mp_call_function_0(link_disconnected_cb);
        engine_link_on_just_disconnected();
    }else if(was_connected == false && connected == true){  // Was disconnected now we aren't, connected
        if(link_connected_cb != mp_const_none) mp_call_function_0(link_connected_cb);
        engine_link_on_just_connected();
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
    // Run this for cases where someone is using a blocking loop
    engine_link_module_task();

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
    DESC: Starts link communication and starts discovery process and will need to be restarted if the device senses it gets disconnected. Disables keyboard interrupt (makes connecting to editors impossible) until {ref_link:engine_link_stop} is called or until the device senses it has been disconnected.
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
    DESC: Stops link communication and makes the Thumby a normal USB device again. Reenables keyboard interrupt that was disabled by {ref_link:engine_link_start}.
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
    DESC: Provided a function, this function will be called when the device connects to another device
    PARAM:  [type=function] [name=connected_cb] [value=function]
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
    DESC: Provided a function, this function will be called when the device disconnects from another device
    PARAM:  [type=function] [name=disconnected_cb] [value=function]
    RETURN: None
*/ 
static mp_obj_t engine_link_module_set_disconnected_cb(mp_obj_t disconnected_cb){
    link_disconnected_cb = disconnected_cb;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_link_module_set_disconnected_cb_obj, engine_link_module_set_disconnected_cb);


/*  --- doc ---
    NAME: send
    ID: engine_link_send
    DESC: Provided a `bytearray`, send up to `count`, `len(send_buffer)`, or `len(send_buffer)-offset` (whichever is smallest and passed). Sends bytes starting at `0` by default or from `offset` if passed. The actual number of bytes sent is returned. If the send buffer is larger than 256 bytes, the extra data is not sent.
    PARAM:  [type=bytearray] [name=send_buffer] [value=bytearray]
    PARAM:  [type=int]       [name=count]       [value=int (optional)]
    PARAM:  [type=int]       [name=offset]      [value=int (optional)]
    RETURN: numbers of bytes sent (int)
*/ 
static mp_obj_t engine_link_module_send(size_t n_args, const mp_obj_t *args){
    if(n_args > 3){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Expected 1 ~ 3 arguments, got %d`"), n_args);
    }

    mp_obj_array_t *send_buffer = NULL;
    uint32_t count = 0;
    uint32_t offset = 0;

    // bytearray
    if(mp_obj_is_type(args[0], &mp_type_bytearray) == false){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: First argument is expected to be a `bytearray`, got `%s`"), mp_obj_get_type_str(args[0]));
    }else{
        send_buffer = args[0];
    }

    // count
    if(n_args >= 2){
        if(mp_obj_is_int(args[1]) == false){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Second argument is expected to be an `int`, got `%s`"), mp_obj_get_type_str(args[1]));
        }else{
            count = mp_obj_get_int(args[1]);
        }
    }else{
        // If count not passed, send entire buffer
        count = send_buffer->len;
    }

    // offset
    if(n_args == 3){
        if(mp_obj_is_int(args[2]) == false){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Third argument is expected to be an `int`, got `%s`"), mp_obj_get_type_str(args[2]));
        }else{
            offset = mp_obj_get_int(args[2]);
        }
    }
    
    // Check that offset is less than the length of the buffer overall
    if(offset >= send_buffer->len){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Offset is %d which is larger than or equal to the size of the send buffer %d"), offset, send_buffer->len);
    }

    // Figure out which of the three is smallest and send
    count = min3(count, send_buffer->len, send_buffer->len-offset);

    // Get the actual number of bytes sent/placed into the TX buffer
    uint32_t sent_count = engine_link_send((uint8_t*)send_buffer->items, count, offset);

    // Run the task first (for host) to get data flowing
    engine_link_module_task();

    return mp_obj_new_int(sent_count);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_link_module_send_obj, 1, 3, engine_link_module_send);


/*  --- doc ---
    NAME: read
    ID: engine_link_read
    DESC: Read up to `count` or `engine_link.available()`, whichever is smallest. Allocates and returns a new `bytearray` if 1 or more bytes to read. If data is not read fast enough and `engine_link.available()` reaches 511 and new data is received, the oldest data is overwritten. 
    PARAM:  [type=int]       [name=count]       [value=int (optional)]
    RETURN: None if no bytes to read otherwise `bytearray`
*/ 
static mp_obj_t engine_link_module_read(mp_obj_t count_obj){
    // Decode passed count and see what's ready to read
    uint32_t count = mp_obj_get_int(count_obj);
    uint32_t available = engine_link_available();

    // Return none if nothing available to read or count 0
    if(available == 0 || count == 0){
        return mp_const_none;
    }

    // See which is smallest
    uint32_t length = min(count, available);

    // Create new bytearray since we have at least one byte
    mp_obj_array_t *array = m_new_obj(mp_obj_array_t);
    array->base.type = &mp_type_bytearray;
    array->typecode = BYTEARRAY_TYPECODE;
    array->free = 0;
    array->len = length;
    array->items = m_new(byte, array->len);

    // Read and return array
    engine_link_read_into((uint8_t*)array->items, length, 0);

    // Run the task first (for host) to get data flowing
    engine_link_module_task();

    return array;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_link_module_read_obj, engine_link_module_read);


/*  --- doc ---
    NAME: read_into
    ID: engine_link_read_into
    DESC: Read up to `count`, `engine_link.available()` or `len(buffer)`, whichever is smallest into `buffer` starting at `0` or from `offset` if passed. If data is not read fast enough and `engine_link.available()` reaches 511 and new data is received, the oldest data is overwritten. 
    PARAM:  [type=bytearray] [name=read_buffer] [value=bytearray]
    PARAM:  [type=int]       [name=count]       [value=int (optional)]
    PARAM:  [type=int]       [name=offset]      [value=int (optional)]
    RETURN: Number of bytes read into `buffer` (int)
*/ 
static mp_obj_t engine_link_module_read_into(size_t n_args, const mp_obj_t *args){
    if(n_args > 3){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Expected 1 ~ 3 arguments, got %d`"), n_args);
    }

    mp_obj_array_t *read_buffer = NULL;
    uint32_t count = 0;
    uint32_t offset = 0;

    // bytearray
    if(mp_obj_is_type(args[0], &mp_type_bytearray) == false){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: First argument is expected to be a `bytearray`, got `%s`"), mp_obj_get_type_str(args[0]));
    }else{
        read_buffer = args[0];
    }

    // count
    if(n_args >= 2){
        if(mp_obj_is_int(args[1]) == false){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Second argument is expected to be an `int`, got `%s`"), mp_obj_get_type_str(args[1]));
        }else{
            count = mp_obj_get_int(args[1]);
        }
    }else{
        // If count not passed, read all available
        count = engine_link_available();
    }

    // offset
    if(n_args == 3){
        if(mp_obj_is_int(args[2]) == false){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Third argument is expected to be an `int`, got `%s`"), mp_obj_get_type_str(args[2]));
        }else{
            offset = mp_obj_get_int(args[2]);
        }
    }
    
    // Check that offset is less than the length of the buffer overall
    if(offset >= read_buffer->len){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineLink: ERROR: Offset is %d which is larger than or equal to the size of the read buffer %d"), offset, read_buffer->len);
    }

    // Figure out which of the three is smallest and read
    count = min3(count, read_buffer->len, read_buffer->len-offset);
    engine_link_read_into((uint8_t*)read_buffer->items, count, offset);

    // Run the task first (for host) to get data flowing
    engine_link_module_task();

    return mp_obj_new_int(count);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_link_module_read_into_obj, 1, 3, engine_link_module_read_into);


/*  --- doc ---
    NAME: available
    ID: engine_link_available
    DESC: Returns the number of bytes available to read from the internal 511 byte buffer. If after 511 bytes more data is sent to the full internal buffer, those extra bytes overwrite the oldest data (ringbuffer).
    RETURN: Number of bytes available to read (int)
*/
static mp_obj_t engine_link_module_available(){
    // Run the task first (for host) to get data flowing
    engine_link_module_task();

    return mp_obj_new_int(engine_link_available());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_available_obj, engine_link_module_available);


/*  --- doc ---
    NAME: clear_send
    ID: engine_link_clear_send
    DESC: Clears any bytes that were queued to be sent (clears USB TX fifo without sending)
    RETURN: mp_const_none
*/
static mp_obj_t engine_link_module_clear_send(){
    engine_link_clear_send();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_clear_send_obj, engine_link_module_clear_send);


/*  --- doc ---
    NAME: clear_read
    ID: engine_link_clear_read
    DESC: Clears any bytes that were queued to be read (clears both the internal ring buffer and USB RX fifo)
    RETURN: mp_const_none
*/
static mp_obj_t engine_link_module_clear_read(){
    engine_link_clear_read();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_clear_read_obj, engine_link_module_clear_read);


/*  --- doc ---
    NAME: is_started
    ID: engine_link_is_started
    DESC: Returns if {ref_link:engine_link_start} has been called
    RETURN: True or False
*/
static mp_obj_t engine_link_module_is_started(){
    return mp_obj_new_bool(engine_link_is_started());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_is_started_obj, engine_link_module_is_started);


/*  --- doc ---
    NAME: is_host
    ID: engine_link_is_host
    DESC: When not connected, always returns `False`. If connected, returns `True` if the unit is acting as the USB host, otherwise returns `False` if acting as a USB device.
    RETURN: True or False
*/
static mp_obj_t engine_link_module_is_host(){
    if(engine_link_connected() == false){
        return mp_obj_new_bool(false);
    }

    return mp_obj_new_bool(engine_link_is_host());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_link_module_is_host_obj, engine_link_module_is_host);


/*  --- doc ---
    NAME: engine_link
    ID: engine_link
    DESC: Module for sending data from one Thumby Color to another
    ATTR: [type=function]   [name={ref_link:engine_link_connected}]             [value=function] 
    ATTR: [type=function]   [name={ref_link:engine_link_start}]                 [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_stop}]                  [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_set_connected_cb}]      [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_set_disconnected_cb}]   [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_send}]                  [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_read}]                  [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_read_into}]             [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_available}]             [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_clear_send}]            [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_clear_read}]            [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_is_started}]            [value=function]
    ATTR: [type=function]   [name={ref_link:engine_link_is_host}]               [value=function]
*/ 
static const mp_rom_map_elem_t engine_link_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_link) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_connected), (mp_obj_t)&engine_link_module_connected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_link_module_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&engine_link_module_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_connected_cb), (mp_obj_t)&engine_link_module_set_connected_cb_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_disconnected_cb), (mp_obj_t)&engine_link_module_set_disconnected_cb_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&engine_link_module_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&engine_link_module_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_into), (mp_obj_t)&engine_link_module_read_into_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_available), (mp_obj_t)&engine_link_module_available_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear_send), (mp_obj_t)&engine_link_module_clear_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear_read), (mp_obj_t)&engine_link_module_clear_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_started), (mp_obj_t)&engine_link_module_is_started_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_host), (mp_obj_t)&engine_link_module_is_host_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_link_globals, engine_link_globals_table);

const mp_obj_module_t engine_link_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_link_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_link, engine_link_user_cmodule);