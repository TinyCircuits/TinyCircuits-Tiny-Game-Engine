#ifndef BITS_H
#define BITS_H

#include <stdint.h>

#define BIT_SET_TRUE(number, bitindex) number = number | ((uint8_t)1 < bitindex)
#define BIT_SET_FALSE(number, bitindex) number = number & ~((uint8_t)1 < bitindex)
#define BIT_GET(number, bitindex) number ^ ((uint8_t)1 << bitindex)

#endif  // BITS_H