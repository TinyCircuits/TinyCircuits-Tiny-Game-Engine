#include "engine_fault_report.h"

#include "debug/debug_print.h"
#include "draw/engine_display_draw.h"
#include "draw/engine_shader.h"
#include "display/engine_display.h"
#include "math/engine_math.h"
#include "resources/engine_font_resource.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Defined in engine_display_common.c
extern uint16_t *active_screen_buffer;


void engine_fault_report(uint32_t lr, uint32_t pc){
    // https://wbk.one/%2Farticle%2F6%2Fdebugging-arm-without-a-debugger-3-printing-stack-trace#:~:text=pc%20(program%20counter)
    ENGINE_PRINTF("HARD_FAULT ERROR: LR=%ld, PC=%ld\n", lr, pc);

    engine_draw_fill_color(0b0000000000011111, active_screen_buffer);

    char message[100] = { 0 };
    int len = snprintf(message, 75, "ERROR: HARD_FAULT\nLR: %x\nPC: %x\nRESTART DEVICE\n\n:(", (int)lr, (int)pc);

    mp_obj_str_t text = {
        .base.type = &mp_type_str,
        .len = len,
        .hash = 0,
        .data = (byte*)message
    };

    engine_draw_text(&default_font, &text, 64.0f, 64.0f, 128.0f, 128.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.0f, 1.0f, engine_get_builtin_shader(EMPTY_SHADER));

    engine_display_send();
}
