/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Core engine definitions.
 *
 * This header pulls in various system headers and headers from the lib
 * directory that are commonly used throughout the engine to avoid having to
 * have the same includes in most files. With the exception of the headers this
 * file includes, any header which does not include any other engine headers
 * should include this one.
 */

#ifndef ORION_CORE_DEFS_H
#define ORION_CORE_DEFS_H

#include "lib/error.h"
#include "lib/log.h"
#include "lib/version.h"

#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>

#endif /* ORION_CORE_DEFS_H */
