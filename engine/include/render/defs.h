/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Renderer definitions.
 */

#pragma once

#include "core/defs.h"

/** Rendering path enumeration. */
enum class RenderPath {
    kForward,                       /**< Forward rendering. */
    kDeferred,                      /**< Deferred lighting. */
};
