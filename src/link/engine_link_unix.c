#include "engine_link_unix.h"


bool engine_link_connected(){
    return false;
}


void engine_link_task(){

}


void engine_link_start(){
    
}


void engine_link_stop(){
    
}


void engine_link_on_just_connected(){

}


void engine_link_on_just_disconnected(){
    engine_link_stop();
}


void engine_link_send(const uint8_t *send_buffer, uint32_t count, uint32_t offset){
    
}


void engine_link_read_into(uint8_t *buffer, uint32_t count, uint32_t offset){
    
}


uint32_t engine_link_available(){
    return 0;
}


void engine_link_clear_send(){

}


void engine_link_clear_read(){
    
}