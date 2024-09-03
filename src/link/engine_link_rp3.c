#include "engine_link_rp3.h"

#include "tusb.h"
#include "host/usbh_pvt.h"
#include "host/usbh_pvt.h"
#include "device/usbd_pvt.h"
#include "device/usbd_pvt.h"
#include "class/cdc/cdc_host.h"
#include "class/cdc/cdc_device.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/stream.h"
#include "py/ringbuf.h"

#include "io/engine_io_rp3.h"
#include "math/engine_math.h"


bool started = false;   // Is discovery started (if so, will flip between device/host when not connected)
bool is_host = false;   // Are we acting as a USB host

// // `engine_link.start()` has been called and we're connected as a device
// // to a host, replace the device driver `xfer_cb` with a custom one to
// // intercept MicroPython from consuming the data before us
// // src/class/cdc/cdc_device.c
// bool custom_device_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes){
//     engine_io_rp3_rumble(1.0f);
//     return true;
// }


// // src/class/cdc/cdc_host.c
// bool custom_host_xfer_cb(uint8_t daddr, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes){
//     engine_io_rp3_rumble(1.0f);
//     return true;
// }



// static usbh_class_driver_t custom_usbh_class_driver = {
//     .name       = "CDC",
//     .init       = cdch_init,
//     .deinit     = cdch_deinit,
//     .open       = cdch_open,
//     .set_config = cdch_set_config,
//     .xfer_cb    = cdch_xfer_cb,
//     .close      = cdch_close
// };


// tu_static usbd_class_driver_t custom_usbd_driver = {
//     .name             = "CDC",
//     .init             = cdcd_init,
//     .deinit           = cdcd_deinit,
//     .reset            = cdcd_reset,
//     .open             = cdcd_open,
//     .control_xfer_cb  = cdcd_control_xfer_cb,
//     .xfer_cb          = cdcd_xfer_cb,
//     .sof              = NULL
// };


// usbh_class_driver_t const* usbh_app_driver_get_cb(uint8_t* driver_count){
//     return &custom_usbh_class_driver;
// }


// usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t* driver_count){
//     return &custom_usbd_driver;
// }


// Related to clipping between being a device or host during
// discovery (when `start` is true). Need to flip since the
// units could have started disconnected
const uint32_t discovery_flip_ticks_max_base = 75000;  
const uint32_t discovery_flip_ticks_max_rand = 20000;
uint32_t discovery_flip_ticks_max = discovery_flip_ticks_max_base + discovery_flip_ticks_max_rand;
uint32_t discovery_flip_ticks_count = 0;


// When a device connects to us this is called, track that
// index as it is the only reference into the tusb list of
// devices that we get
uint8_t mounted_device_daddr = 0;
uint8_t mounted_device_cdc_daddr = 0;
void tuh_mount_cb(uint8_t daddr){
    mounted_device_daddr = daddr;
}


// When a CDC device connects to us, track that,
// only time we can get its device index
void tuh_cdc_mount_cb(uint8_t idx){
    mounted_device_cdc_daddr = idx;
    engine_io_rp3_set_indicator(true);
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


// void engine_link_on_just_connected(){
//     if(is_host){

//     }else{
//         custom_usbd_driver.xfer_cb = custom_device_xfer_cb;
//     }
// }


// void engine_link_on_just_disconnected(){
//     if(is_host){

//     }else{
//         custom_usbd_driver.xfer_cb = cdcd_xfer_cb;
//     }
// }


void engine_link_send(const void *buffer, uint32_t bufsize){
    // Don't try to send if not connected to anything
    if(!engine_link_connected()){
        return;
    }

    // Send
    if(is_host){
        tuh_cdc_write(mounted_device_cdc_daddr, buffer, bufsize);
        tuh_cdc_write_flush(mounted_device_cdc_daddr);
    }else{
        tud_cdc_write(buffer, bufsize);
        tud_cdc_write_flush();
    }
}


void engine_link_recv(){

}


uint32_t engine_link_available(){
    if(is_host){
        return tuh_cdc_read_available(mounted_device_cdc_daddr);
    }else{
        // ports/rp2/mphalport.h is included through py/mphal.h -> <mphalport.h>
        return ringbuf_avail(&stdin_ringbuf);
    }
}