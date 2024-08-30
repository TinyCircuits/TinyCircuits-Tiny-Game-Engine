#ifndef ENGINE_LINK_RP3_H
#define ENGINE_LINK_RP3_H

#include <stdbool.h>
#include <stdint.h>

bool engine_link_connected();
void engine_link_task();
void engine_link_start();
void engine_link_stop();

void engine_link_send(const void *buffer, uint32_t bufsize);
void engine_link_recv();
uint32_t engine_link_available();


#endif  // ENGINE_LINK_RP3_H