#ifndef ENGINE_DISPLAY_H
#define ENGINE_DISPLAY_H


// Initialize the screen
void engine_display_init();

void engine_display_apply_brightness(float brightness);
float engine_display_get_brightness();

// Send the active frame to the display/screen
// and switch the dual buffers/active buffer
void engine_display_send();


#endif  // ENGINE_DISPLAY_H