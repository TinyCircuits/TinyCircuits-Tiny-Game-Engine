#include "engine_file.h"
#include <stdlib.h>
#include <fcntl.h>
#include "debug/debug_print.h"

// Because of the vast differences between file operations on RP2 and UNIX,
// there are separate translation units (.c files) for each platform.
int file_fd;



void engine_file_open(const char *filename){
    file_fd = open(filename, O_RDONLY);
}


void engine_file_close(){
    close(file_fd);
}


void engine_file_read(void *buffer, uint32_t size){
    int read_count = read(file_fd, buffer, size);
}


uint8_t engine_file_get_u8(uint32_t u8_byte_offset){
    uint8_t the_u8_byte = 0;
    lseek(file_fd, u8_byte_offset, SEEK_SET);
    int read_count = read(file_fd, &the_u8_byte, 1);
    return the_u8_byte;
}


uint16_t engine_file_get_u16(uint32_t u8_byte_offset){
    uint16_t the_u16_byte = 0;
    lseek(file_fd, u8_byte_offset, SEEK_SET);
    int read_count = read(file_fd, &the_u16_byte, 2);
    return the_u16_byte;
}


uint32_t engine_file_get_u32(uint32_t u8_byte_offset){
    uint32_t the_u32_byte = 0;
    lseek(file_fd, u8_byte_offset, SEEK_SET);
    int read_count = read(file_fd, &the_u32_byte, 4);
    return the_u32_byte;
}


void engine_fast_cache_file_init(engine_fast_cache_file_t *cache_file, const char *filename){
    file_fd = open(filename, O_RDONLY);

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