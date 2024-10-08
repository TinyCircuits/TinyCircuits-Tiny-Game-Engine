#ifndef BITS_H
#define BITS_H

#include <stdint.h>

#define BIT_SET(number, bitindex, state) number = (number & ~((uint32_t)1 << bitindex)) | ((uint32_t)state << bitindex)
#define BIT_SET_TRUE(number, bitindex) number = number | ((uint32_t)1 << bitindex)
#define BIT_SET_FALSE(number, bitindex) number = number & ~((uint32_t)1 << bitindex)
#define BIT_GET(number, bitindex) number & ((uint32_t)1 << bitindex)

#endif  // BITS_H