#include "engine_bit_collection.h"
#include "py/obj.h"
#include "py/misc.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>


void engine_bit_collection_create(engine_bit_collection_t *collection, uint32_t bit_count){
    collection->byte_count = (uint32_t)ceilf((float)bit_count / 8.0f);
    collection->bit_collection = malloc(sizeof(uint8_t) * collection->byte_count);
    collection->dirty = false;
}


bool engine_bit_collection_get(engine_bit_collection_t *collection, uint32_t bit_index){
    uint32_t byte_index = bit_index >> 3;               // Divide by 8
    uint8_t bit_index_in_byte = (8 - 1) & bit_index;    // Remainder after divide by 8 (https://stackoverflow.com/a/74766453)

    // Return either a number greater than 0 (thus getting cast to 1), or 0
    return collection->bit_collection[byte_index] & (0b00000001 << bit_index_in_byte);
}


void engine_bit_collection_set(engine_bit_collection_t *collection, uint32_t bit_index){
    uint32_t byte_index = bit_index >> 3;               // Divide by 8
    uint8_t bit_index_in_byte = (8 - 1) & bit_index;    // Remainder after divide by 8 (https://stackoverflow.com/a/74766453)

    collection->bit_collection[byte_index] |= (0b00000001 << bit_index_in_byte);
    collection->dirty = true;
}


void engine_bit_collection_erase(engine_bit_collection_t *collection){
    // Don't want to clear this if there's no reason to
    if(collection->dirty){
        memset(collection->bit_collection, 0, collection->byte_count);
    }
}