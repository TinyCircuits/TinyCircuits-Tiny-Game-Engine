#include "engine_file.h"
#include <stdlib.h>
#include <fcntl.h>
#include "debug/debug_print.h"

// Because of the vast differences between file operations on RP2 and UNIX,
// there are separate translation units (.c files) for each platform.


void engine_fast_cache_file_init(engine_fast_cache_file_t *cache_file, const char *filename){
    int file_fd = open(filename, O_RDONLY);

    off_t file_size = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);
    cache_file->file_data = malloc(file_size);

    int read_count = read(file_fd, cache_file->file_data, file_size);
    close(file_fd);

    ENGINE_INFO_PRINTF("Engine File Unix: cached %lu bytes directly in ram", read_count);
}


void engine_fast_cache_file_deinit(engine_fast_cache_file_t *cache_file){
    free(cache_file->file_data);
}


uint8_t engine_fast_cache_file_get_u8(engine_fast_cache_file_t *cache_file, uint32_t offset_u8){
    return cache_file->file_data[offset_u8];
}


uint16_t engine_fast_cache_file_get_u16(engine_fast_cache_file_t *cache_file, uint32_t offset_u16){
    return ((uint16_t*)(cache_file->file_data))[offset_u16];
}