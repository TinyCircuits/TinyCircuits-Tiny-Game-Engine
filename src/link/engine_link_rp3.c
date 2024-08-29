#include "engine_link_rp3.h"
#include "tusb.h"
#include "io/engine_io_rp3.h"


bool is_host = false;

const uint32_t try_be_host_ticks_max = 35000;
uint32_t try_be_host_ticks_count = 0;

uint8_t mounted_daddr = 0;


void tuh_mount_cb(uint8_t daddr){
    mounted_daddr = daddr;
    engine_io_rp3_set_indicator(true);
}


void tuh_umount_cb(uint8_t daddr){
    engine_io_rp3_set_indicator(false);
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
        }else{
            tud_deinit(0);
            tuh_init(0);
            is_host = true;
        }
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