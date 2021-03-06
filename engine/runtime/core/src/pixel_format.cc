/*
 * Copyright (C) 2015-2017 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Pixel format information.
 *
 * TODO:
 *  - If we start adding more bits of information per pixel format, we should
 *    add an info table rather than casing each format per function.
 */

#include "core/pixel_format.h"

/** Check if a format is a colour format.
 * @param format        Format to check.
 * @return              Whether the format is a colour format. */
bool PixelFormat::isColour(const PixelFormat format) {
    return !isDepth(format);
}

/** Check if a format is an sRGB format.
 * @param format        Format to check.
 * @return              Whether the format is an sRGB format. */
bool PixelFormat::isSRGB(const PixelFormat format) {
    switch (format) {
        case kR8G8B8A8sRGB:
        case kB8G8R8A8sRGB:
            return true;
        default:
            return false;
    }
}

/** Check if a format is a floating point colour format.
 * @param format        Format to check.
 * @return              Whether the format is a floating point colour format. */
bool PixelFormat::isFloat(const PixelFormat format) {
    switch (format) {
        case kFloatR16G16B16A16:
        case kFloatR16G16B16:
        case kFloatR16G16:
        case kFloatR16:
        case kFloatR32G32B32A32:
        case kFloatR32G32B32:
        case kFloatR32G32:
        case kFloatR32:
            return true;
        default:
            return false;
    }
}

/** Check if a format is a depth format.
 * @param format        Format to check.
 * @return              Whether the format is a depth format. */
bool PixelFormat::isDepth(const PixelFormat format) {
    switch (format) {
        case kDepth16:
        case kDepth32:
        case kDepth32Stencil8:
            return true;
        default:
            return false;
    }
}

/** Check if a format is a depth/stencil format.
 * @param format        Format to check.
 * @return              Whether the format is a depth format. */
bool PixelFormat::isDepthStencil(const PixelFormat format) {
    switch (format) {
        case kDepth32Stencil8:
            return true;
        default:
            return false;
    }
}

/** Get the number of bytes per pixel for a pixel format.
 * @param format        Format to get for.
 * @return              Number of bytes per pixel using the given format. */
size_t PixelFormat::bytesPerPixel(const PixelFormat format) {
    switch (format) {
        case kFloatR32G32B32A32:
            return 16;
        case kFloatR32G32B32:
            return 12;
        case kFloatR16G16B16A16:
        case kFloatR32G32:
        case kDepth32Stencil8:
            return 8;
        case kFloatR16G16B16:
            return 6;
        case kR8G8B8A8:
        case kR8G8B8A8sRGB:
        case kB8G8R8A8:
        case kB8G8R8A8sRGB:
        case kR10G10B10A2:
        case kFloatR16G16:
        case kFloatR32:
        case kDepth32:
            return 4;
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

/** Given a pixel format, get a sRGB equivalent of it.
 * @param format        Format to convert. */
PixelFormat PixelFormat::getSRGBEquivalent(PixelFormat format) {
    switch (format) {
        case PixelFormat::kR8G8B8A8:
            return PixelFormat::kR8G8B8A8sRGB;
        case PixelFormat::kB8G8R8A8:
            return PixelFormat::kB8G8R8A8sRGB;
        default:
            return format;
    }
}

/** Given a pixel format, get a non-sRGB equivalent of it.
 * @param format        Format to convert. */
PixelFormat PixelFormat::getNonSRGBEquivalent(PixelFormat format) {
    switch (format) {
        case PixelFormat::kR8G8B8A8sRGB:
            return PixelFormat::kR8G8B8A8;
        case PixelFormat::kB8G8R8A8sRGB:
            return PixelFormat::kB8G8R8A8;
        default:
            return format;
    }
}

