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
#include "py/misc.h"

#include "shared/readline/readline.h"

#include "io/engine_io_rp3.h"
#include "math/engine_math.h"


// Mimic ports/rp2/mphalport.c ring buffer for device for host
static uint8_t stdin_host_ringbuf_array[512];
ringbuf_t stdin_host_ringbuf = { stdin_host_ringbuf_array, sizeof(stdin_host_ringbuf_array) };


void ringbuf_clear(ringbuf_t *ringbuf){
    while(ringbuf_avail(ringbuf) > 0){
        ringbuf_get(ringbuf);
    }
}


bool started = false;   // Is discovery started (if so, will flip between device/host when not connected)
bool is_host = false;   // Are we acting as a USB host


// Related to clipping between being a device or host during
// discovery (when `start` is true). Need to flip since the
// units could have started disconnected
const uint32_t discovery_flip_ticks_max_base = 75000;  
const uint32_t discovery_flip_ticks_max_rand = 100000;
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
}


// Mimic shared/tinyusb/mp_usbd_cdc.c device handling
// of CDC data into a ringbuffer but for host
void tuh_cdc_rx_cb(uint8_t idx){
    for(uint32_t bytes_avail=tuh_cdc_read_available(idx); bytes_avail>0; --bytes_avail) {
        uint8_t data = 0;
        tuh_cdc_read(idx, &data, 1);
        ringbuf_put(&stdin_host_ringbuf, data);
    }
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
    // If we're acting as a host, run the tusb host task
    if(is_host){
        tuh_task();
    }

    // If we're connected or something is connected to us,
    // do not run the device/host flipping code after this
    if(started == false || engine_link_connected()){
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
    mp_hal_set_interrupt_char(-1);
    started = true;
}


// Stop USB/link discovery and go back to being a USB device
void engine_link_stop(){
    mp_hal_set_interrupt_char(CHAR_CTRL_C);
    started = false;
    engine_link_switch_to_device();
}


void engine_link_on_just_connected(){

}


void engine_link_on_just_disconnected(){
    engine_link_stop();
}


uint32_t engine_link_send(const uint8_t *send_buffer, uint32_t count, uint32_t offset){
    // Don't try to send if not connected to anything
    if(!engine_link_connected()){
        return 0;
    }

    uint32_t sent_count = 0;

    // Send
    if(is_host){
        sent_count = tuh_cdc_write(mounted_device_cdc_daddr, send_buffer+offset, count);
        tuh_cdc_write_flush(mounted_device_cdc_daddr);
    }else{
        sent_count = tud_cdc_write(send_buffer+offset, count);
        tud_cdc_write_flush();
    }

    return sent_count;
}


// This is an unsafe raw function, functions that call this are expected
// to make sure reads will not go out of bounds and that there is enough
// data in the ring buffers
void engine_link_read_into(uint8_t *buffer, uint32_t count, uint32_t offset){
    ringbuf_t *ringbuf = NULL;

    if(is_host){
        ringbuf = &stdin_host_ringbuf;
    }else{
        ringbuf = &stdin_ringbuf;
    }

    for(uint32_t i=0; i<count; i++){
        buffer[offset+i] = ringbuf_get(ringbuf);
    }
}


uint32_t engine_link_available(){
    if(is_host){
        return ringbuf_avail(&stdin_host_ringbuf);
    }else{
        // ports/rp2/mphalport.h is included through py/mphal.h -> <mphalport.h>
        return ringbuf_avail(&stdin_ringbuf);
    }
}


void engine_link_clear_send(){
    if(is_host){
        tuh_cdc_write_clear(mounted_device_cdc_daddr);
    }else{
        tud_cdc_write_clear();
    }
}


void engine_link_clear_read(){
    if(is_host){
        tuh_cdc_read_clear(mounted_device_cdc_daddr);
        ringbuf_clear(&stdin_host_ringbuf);
    }else{
        tud_cdc_read_flush();
        ringbuf_clear(&stdin_ringbuf);
    }
}


bool engine_link_is_started(){
    return started;
}


bool engine_link_is_host(){
    return is_host;
}