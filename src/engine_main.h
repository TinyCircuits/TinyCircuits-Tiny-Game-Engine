#ifndef ENGINE_MAIN_H
#define ENGINE_MAIN_H

void engine_main_raise_if_not_initialized();
void engine_main_reset();


#if defined(__unix__)
    #define FILESYSTEM_ROOT_MAX_LEN 512
#else
    #define FILESYSTEM_ROOT_MAX_LEN 2
#endif

extern char filesystem_root[FILESYSTEM_ROOT_MAX_LEN];


#endif  // ENGINE_MAIN_H