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
 * @brief               Pixel format definitions.
 *
 * TODO:
 *  - Format information functions, supported pixel format information in
 *    GPUManager.
 */

#pragma once

#include "core/core.h"
#include "core/hash.h"

/** Pixel format definitions. */
struct PixelFormat {
    enum Impl {
        /**< Unknown format. */
        kUnknown,

        /**
         * Colour formats.
         *
         * Note these are all given in the order of elements in memory,
         * independent of endianness.
         */
        kR8G8B8A8,              /**< RGBA, unsigned normalized, 8 bits per component. */
        kR8G8B8A8sRGB,          /**< RGBA, unsigned normalized, 8 bits per component, sRGB. */
        kR8G8,                  /**< RG, unsigned normalized, 8 bits per component. */
        kR8,                    /**< R, unsigned normalized, 8 bits per component. */
        kB8G8R8A8,              /**< BGRA, unsigned normalized, 8 bits per component. */
        kB8G8R8A8sRGB,          /**< BGRA, unsigned normalized, 8 bits per component, sRGB. */
        kR10G10B10A2,           /**< RGBA, unsigned normalized, 10 bits RGB, 2 bits A. */
        kFloatR16G16B16A16,     /**< RGBA, float, 16 bits per component. */
        kFloatR16G16B16,        /**< RGB, float, 16 bits per component. */
        kFloatR16G16,           /**< RG, float, 16 bits per component. */
        kFloatR16,              /**< R, float, 16 bits per component. */
        kFloatR32G32B32A32,     /**< RGBA, float, 32 bits per component. */
        kFloatR32G32B32,        /**< RGB, float, 32 bits per component. */
        kFloatR32G32,           /**< RG, float, 32 bits per component. */
        kFloatR32,              /**< R, float, 32 bits per component. */

        /** Depth/stencil formats. */
        kDepth16,               /**< Depth, 16 bits. */
        kDepth32,               /**< Depth, 32 bits. */
        kDepth32Stencil8,       /**< Depth/stencil, 24 bits/8 bits. */

        /** Number of pixel formats. */
        kNumFormats,
    };

    constexpr PixelFormat() : m_value(kUnknown) {}
    constexpr PixelFormat(Impl value) : m_value(value) {}
    constexpr operator Impl() const { return m_value; }

    static bool isColour(PixelFormat format);
    static bool isSRGB(PixelFormat format);
    static bool isFloat(PixelFormat format);
    static bool isDepth(PixelFormat format);
    static bool isDepthStencil(PixelFormat format);

    static size_t bytesPerPixel(PixelFormat format);

    static PixelFormat getSRGBEquivalent(PixelFormat format);
    static PixelFormat getNonSRGBEquivalent(PixelFormat format);

    /** Get a hash from a pixel format. */
    friend size_t hashValue(PixelFormat format) {
        return hashValue(format.m_value);
    }
private:
    Impl m_value;
};
