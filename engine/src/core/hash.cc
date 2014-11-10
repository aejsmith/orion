/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Hash functions.
 */

#include "core/hash.h"

#include "../../../3rdparty/cityhash/city.cc"

/** Hash a block of memory.
 * @param buf           Buffer to hash.
 * @param len           Length of block.
 * @return              Generated hash. */
size_t hashMem(const void *buf, size_t len) {
    #if ORION_64BIT
        return CityHash64(reinterpret_cast<const char *>(buf), len);
    #else
        return CityHash32(reinterpret_cast<const char *>(buf), len);
    #endif
}
