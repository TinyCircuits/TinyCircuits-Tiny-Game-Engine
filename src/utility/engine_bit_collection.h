#ifndef ENGINE_BIT_COLLECTION_H
#define ENGINE_BIT_COLLECTION_H

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t *bit_collection;
    uint32_t byte_count;
}engine_bit_collection_t;


// Initializes a new bit collection based on the provided `bit_count`
void engine_bit_collection_create(engine_bit_collection_t *collection, uint32_t bit_count);

// Gets whether or not the bit at `bit_index` is true or false
bool engine_bit_collection_get(engine_bit_collection_t *collection, uint32_t bit_index);

// Sets the bit at `bit_index` to true
void engine_bit_collection_set(engine_bit_collection_t *collection, uint32_t bit_index);

// Erases all bits in collection by setting them to all '0' using memset
void engine_bit_collection_erase(engine_bit_collection_t *collection);


#endif  // ENGINE_BIT_COLLECTION_H