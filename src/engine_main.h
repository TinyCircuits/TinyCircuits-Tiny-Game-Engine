#ifndef ENGINE_MAIN_H
#define ENGINE_MAIN_H

void engine_main_raise_if_not_initialized();
void engine_main_reset();

#if defined(__unix__)
    extern char filesystem_root[1024];
#endif

#endif  // ENGINE_MAIN_H