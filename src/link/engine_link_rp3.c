#include "engine_link_rp3.h"
#include "tusb.h"
#include "io/engine_io_rp3.h"
#include "math/engine_math.h"


bool is_host = false;

const uint32_t try_be_host_ticks_max_base = 50000;
const uint32_t try_be_host_ticks_max_rand = 20000;
uint32_t try_be_host_ticks_max = try_be_host_ticks_max_base + try_be_host_ticks_max_rand;

uint32_t try_be_host_ticks_count = 0;

uint8_t mounted_daddr = 0;


void tuh_mount_cb(uint8_t daddr){
    mounted_daddr = daddr;
}


bool engine_link_connected(){
    bool connected = false;

    // If the device is connected to a host, or if the first
    // device connected to the hos is ready, return true
    if(tud_ready() || tuh_ready(mounted_daddr)){
        connected = true;
    }

    return connected;
}


void engine_link_task(){
    if(is_host){
        tuh_task();
    }else if(!engine_link_connected()){
        tud_connect();
    }

    if(engine_link_connected()){
        return;
    }

    try_be_host_ticks_count++;

    
    if(try_be_host_ticks_count == try_be_host_ticks_max){
        if(is_host){
            tuh_deinit(0);
            tud_init(0);
            tud_connect();
            is_host = false;
            engine_io_rp3_set_indicator(true);
        }else{
            tud_deinit(0);
            tuh_init(0);
            is_host = true;
            engine_io_rp3_set_indicator(false);
        }

        try_be_host_ticks_max = try_be_host_ticks_max_base + engine_math_rand_int(try_be_host_ticks_max_rand);
    }


    if(try_be_host_ticks_count >= try_be_host_ticks_max){
        try_be_host_ticks_count = 0;
    }
}


void engine_link_start(){
    // tud_deinit(0);
}


void engine_link_stop(){
    // tud_init(0);
}