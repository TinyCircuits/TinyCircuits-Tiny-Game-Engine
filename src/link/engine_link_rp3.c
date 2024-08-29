#include "engine_link_rp3.h"
#include "tusb.h"
#include "io/engine_io_rp3.h"
#include "math/engine_math.h"


bool started = false;   // Is discovery started (if so, will flip between device/host when not connected)
bool is_host = false;   // Are we acting as a USB host


// Related to clipping between being a device or host during
// discovery (when `start` is true). Need to flip since the
// units could have started disconnected
const uint32_t discovery_flip_ticks_max_base = 50000;  
const uint32_t discovery_flip_ticks_max_rand = 20000;
uint32_t discovery_flip_ticks_max = discovery_flip_ticks_max_base + discovery_flip_ticks_max_rand;
uint32_t discovery_flip_ticks_count = 0;


// When a device connects to us this is called, track that
// index as it is the only reference into the tusb list of
// devices that we get
uint8_t mounted_device_daddr = 0;
void tuh_mount_cb(uint8_t daddr){
    mounted_device_daddr = daddr;
}


bool engine_link_connected(){
    bool connected = false;

    // If the device is connected to a host, or if the first
    // device connected to the host is ready, return true
    if(tud_ready() || tuh_ready(mounted_device_daddr)){
        connected = true;
    }

    return connected;
}


// Switch back to being a USB device
void engine_link_switch_to_device(){
    tuh_deinit(0);
    tud_init(0);
    tud_connect();
    is_host = false;
}


// Switch back to being a USB host
void engine_link_switch_to_host(){
    tud_deinit(0);
    tuh_init(0);
    is_host = true;
}


// This needs to be called often to perform USB discovery of connected
// devices and to run the tusb host task (MicroPython calls the device
// task when device is inited)
void engine_link_task(){
    // If not started, do not perform discovery or host task
    if(started == false){
        return;
    }

    // If we're acting as a host, run the tusb host task
    if(is_host){
        tuh_task();
    }

    // If we're connected or something is connected to us,
    // do not run the device/host flipping code after this
    if(engine_link_connected()){
        return;
    }

    // Discovery: when `discovery_flip_ticks_count` reaches `discovery_flip_ticks_max`,
    // flip between being a device or host and add random durations for the time we are
    // either
    discovery_flip_ticks_count++;
    
    if(discovery_flip_ticks_count == discovery_flip_ticks_max){
        // Flip
        if(is_host){
            engine_link_switch_to_device();
        }else{
            engine_link_switch_to_host();
        }

        // Set random-ish duration and reset counter
        discovery_flip_ticks_max = discovery_flip_ticks_max_base + engine_math_rand_int(discovery_flip_ticks_max_rand);
        discovery_flip_ticks_count = 0;
    }
}


// Start USB/link discovery
void engine_link_start(){
    started = true;
}


// Stop USB/link discovery and go back to being a USB device
void engine_link_stop(){
    started = false;
    engine_link_switch_to_device();
}