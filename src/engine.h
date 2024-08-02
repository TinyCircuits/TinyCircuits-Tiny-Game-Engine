#ifndef ENGINE_H
#define ENGINE_H

float engine_get_fps_limit_ms();

// Set the core clock to some frequency (does not
// reset UART but does adjust audio playback)
void engine_set_freq(uint32_t hz);

#endif  // ENGINE_H