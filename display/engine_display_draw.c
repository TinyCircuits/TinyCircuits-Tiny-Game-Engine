#include "engine_display_draw.h"
#include "engine_display_common.h"

#include <string.h>

void engine_draw_fill(uint16_t color){
    memset(engine_get_active_screen_buffer(), color, SCREEN_BUFFER_SIZE);
}