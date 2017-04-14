/*
 * Copyright (C) 2015 Alex Smith
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
        case kDepth32Stencil8:
            return 8;
        case kFloatR16G16B16:
            return 6;
        case kR8G8B8A8:
        case kB8G8R8A8:
        case kR10G10B10A2:
        case kFloatR16G16:
        case kFloatR32:
        case kDepth32:
            return 4;
        case kR8G8B8:
        case kB8G8R8:
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
