#include "engine_time_module.h"
#include "debug/debug_print.h"
#include "engine_rtc.h"
#include "py/obj.h"


int engine_time_get(mp_obj_t *output_tuple){
    struct tm rtc;

    // Get time and return if there's an error
    int status = engine_rtc_get_datetime(&rtc);
    if(status != true) return status;

    // https://github.com/micropython/micropython/blob/ba98533454eef5ab5783039f9929351c8f54d005/ports/rp2/machine_rtc.c#L77-L86
    // https://docs.micropython.org/en/latest/library/time.html#time.localtime <- mimicking this format
    mp_obj_t tuple_objs[8] = {
        mp_obj_new_int(rtc.tm_year + 1900),
        mp_obj_new_int(rtc.tm_mon + 1),
        mp_obj_new_int(rtc.tm_mday),
        mp_obj_new_int(rtc.tm_hour),
        mp_obj_new_int(rtc.tm_min),
        mp_obj_new_int(rtc.tm_sec),
        mp_obj_new_int(rtc.tm_wday),
        mp_obj_new_int(rtc.tm_yday),
    };

    *output_tuple = mp_obj_new_tuple(8, tuple_objs);

    return true;
}


int engine_time_set(mp_obj_t input_tuple){
    // Make sure we are dealing with a tuple
    if(mp_obj_is_type(input_tuple, &mp_type_tuple) == false){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineTime: ERROR: Expected a tuple to set the time, got %s"), mp_obj_get_type_str(input_tuple));
    }

    mp_obj_tuple_t *tuple = input_tuple;

    // Make sure the tuple has the correct number of arguments (between 6 and 8)
    if(tuple->len < 6 || tuple->len > 8){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineTime: ERROR: Expected a tuple with 6 ~ 8 elements (year, month, mday, hour, minute, second, wday[unused], yday[unused]) but got %d elements"), tuple->len);
    }

    mp_obj_t *items = tuple->items;

    // Fill out of the `tm` struct with enough information
    // to calculate the rest of the day_of_month and day_of_year
    // information using C libraries
    struct tm rtc;
    rtc.tm_year = mp_obj_get_int(items[0]) - 1900;
    rtc.tm_mon = mp_obj_get_int(items[1]) - 1;
    rtc.tm_mday = mp_obj_get_int(items[2]);
    rtc.tm_hour = mp_obj_get_int(items[3]);
    rtc.tm_min = mp_obj_get_int(items[4]);
    rtc.tm_sec = mp_obj_get_int(items[5]);

    // Do this so that the rest of the information is calculated
    time_t seconds_since_epoch = mktime(&rtc);
    struct tm *rtc_result = localtime(&seconds_since_epoch);

    return engine_rtc_set_datetime(rtc_result);
}


/*  --- doc ---
    NAME: datetime
    ID: datetime
    DESC: Returns a datetime tuple representing the current date down to the second (based on the RTC clock that may lose time if it/Thumby Color fully loses power) 
    PARAM: [type=tuple]   [name=time]  [value=tuple of ints in format (year, month, mday, hour, min, sec, wday{optional & unused}, yday{optional & unused}) (optional)]
    RETURN: RTC_OK, RTC_I2C_ERROR or tuple (year, month, mday, hour, min, sec, wday, yday)
*/
static mp_obj_t engine_time_datetime(size_t n_args, const mp_obj_t *args){
    int status = 0;

    if(n_args == 0){
        mp_obj_t time_tuple = mp_const_none;
        status = engine_time_get(&time_tuple);

        // If no error while getting the time, return the time tuple
        if(status == ENGINE_RTC_OK) return time_tuple;
    }else if(n_args == 1){
        status = engine_time_set(args[0]);
    }else{
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineTime: ERROR: Incorrect number of arguments passed to `datetime()`, expects 0 or 1, got %d"), n_args);
    }

    // Either RTC_OK or RTC_I2C_ERROR at this point
    return mp_obj_new_int(status);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_time_datetime_obj, 0, 1, engine_time_datetime);


/*  --- doc ---
    NAME: is_compromised
    ID: is_compromised
    DESC: Returns True if the RTC clock detected a low voltage that meant it lost power or was very close
    RETURN: True or False
*/
static mp_obj_t engine_time_is_compromised(size_t n_args, const mp_obj_t *args){
    return mp_obj_new_bool(engine_rtc_check_compromised());
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_time_is_compromised_obj, 0, 1, engine_time_is_compromised);


static mp_obj_t engine_time_module_init(){
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_time_module_init_obj, engine_time_module_init);


/*  --- doc ---
    NAME: engine_time
    ID: engine_time
    DESC: Module for doing time related operations in the engine (such as setting and getting the RTC time)
    ATTR: [type=function]   [name={ref_link:datetime}]          [value=function]
    ATTR: [type=function]   [name={ref_link:is_compromised}]    [value=function]
    ATTR: [type=int/enum]   [name=RTC_OK]                       [value=1]
    ATTR: [type=int/enum]   [name=RTC_I2C_ERROR]                [value=-1]
*/
static const mp_rom_map_elem_t engine_time_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_time) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), MP_ROM_PTR(&engine_time_module_init_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_datetime), MP_ROM_PTR(&engine_time_datetime_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_compromised), MP_ROM_PTR(&engine_time_is_compromised_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RTC_OK), MP_ROM_INT(ENGINE_RTC_OK) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RTC_I2C_ERROR), MP_ROM_INT(ENGINE_RTC_I2C_ERROR) },
};
static MP_DEFINE_CONST_DICT (mp_module_engine_time_globals, engine_time_globals_table);


const mp_obj_module_t engine_time_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_time_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_time, engine_time_user_cmodule);
