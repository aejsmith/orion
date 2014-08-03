/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Core engine definitions.
 *
 * This file pulls in commonly used system headers to avoid having to include
 * them everywhere. If a header has no other includes, it should at least
 * include this header.
 */

#ifndef ORION_CORE_DEFS_H
#define ORION_CORE_DEFS_H

#include "lib/error.h"
#include "lib/noncopyable.h"
#include "lib/version.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>

#endif /* ORION_CORE_DEFS_H */
