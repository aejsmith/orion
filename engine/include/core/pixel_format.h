/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Pixel format definitions.
 *
 * @todo                Format information functions, supported pixel format
 *                      information in GPUInterface.
 */

#pragma once

#include "core/core.h"

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
        kR8G8B8,                /**< RGB, unsigned normalized, 8 bits per component. */
        kR8G8,                  /**< RG, unsigned normalized, 8 bits per component. */
        kR8,                    /**< R, unsigned normalized, 8 bits per component. */
        kB8G8R8A8,              /**< BGRA, unsigned normalized, 8 bits per component. */
        kB8G8R8,                /**< BGR, unsigned normalized, 8 bits per component. */
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
        kDepth24,               /**< Depth, 24 bits. */
        kDepth24Stencil8,       /**< Depth/stencil, 24 bits/8 bits. */

        /** Number of pixel formats. */
        kNumFormats,
    };

    constexpr PixelFormat() : m_value(kUnknown) {}
    constexpr PixelFormat(Impl value) : m_value(value) {}
    constexpr operator Impl() const { return m_value; }
private:
    Impl m_value;
};
