#ifndef ENGINE_H
#define ENGINE_H

/*! \mainpage Game Engine Documentation
 *
 * \section intro_sec Introduction
 * This is the main documentation website for the MicroPython based <a href="https://github.com/TinyCircuits/TinyCircuits-Tiny-Game-Engine">TinyCircuits-Tiny-Game-Engine</a>.
 */

float engine_get_fps_limit_ms();

// Set the core clock to some frequency (does not
// reset UART but does adjust audio playback)
void engine_set_freq(uint32_t hz);

#endif  // ENGINE_H