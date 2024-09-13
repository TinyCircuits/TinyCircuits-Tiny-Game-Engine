#ifndef ENGINE_LINK_UNIX_H
#define ENGINE_LINK_UNIX_H

#include <stdbool.h>
#include <stdint.h>

bool engine_link_connected();
void engine_link_task();
void engine_link_start();
void engine_link_stop();

void engine_link_on_just_connected();
void engine_link_on_just_disconnected();

void engine_link_send(const uint8_t *send_buffer, uint32_t count, uint32_t offset);
void engine_link_read_into(uint8_t *buffer, uint32_t count, uint32_t offset);
uint32_t engine_link_available();
void engine_link_clear_send();
void engine_link_clear_read();

bool engine_link_is_host();

#endif  // ENGINE_LINK_UNIX_H