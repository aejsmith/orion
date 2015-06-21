/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Pixel format information.
 *
 * TODO:
 *  - If we start adding more bits of information per pixel format, we should
 *    add an info table rather than casing each format per function.
 */

#include "core/pixel_format.h"

/** Get the number of bytes per pixel for a pixel format.
 * @param format        Format to get for.
 * @return              Number of bytes per pixel using the given format. */
size_t PixelFormat::bytesPerPixel(PixelFormat format) {
    switch (format) {
        case kFloatR32G32B32A32:
            return 16;
        case kFloatR32G32B32:
            return 12;
        case kFloatR16G16B16A16:
        case kFloatR32G32:
            return 8;
        case kFloatR16G16B16:
            return 6;
        case kR8G8B8A8:
        case kB8G8R8A8:
        case kR10G10B10A2:
        case kFloatR16G16:
        case kFloatR32:
        case kDepth24Stencil8:
            return 4;
        case kR8G8B8:
        case kB8G8R8:
        case kDepth24:
            return 3;
        case kR8G8:
        case kFloatR16:
        case kDepth16:
            return 2;
        case kR8:
            return 1;
        default:
            unreachable();
    }
}
