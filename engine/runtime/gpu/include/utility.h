/*
 * Copyright (C) 2016 Alex Smith
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
 * @brief               GPU utility functions.
 */

#pragma once

#include "gpu/defs.h"

namespace GPUUtil {
    /** Calculate the size of a mip level.
     * @param mip           Mip level.
     * @param width         Base level width, set to mip level width.
     * @param height        Base level height, set to mip level height.
     * @param depth         Base level depth, set to mip level depth. */
    static inline void calcMipDimensions(unsigned mip, uint32_t &width, uint32_t &height, uint32_t &depth) {
        width = std::max(width >> mip, 1u);
        height = std::max(height >> mip, 1u);
        depth = std::max(depth >> mip, 1u);
    }

    /** Calculate the size of a mip level.
     * @param mip           Mip level.
     * @param width         Base level width, set to mip level width.
     * @param height        Base level height, set to mip level height. */
    static inline void calcMipDimensions(unsigned mip, uint32_t &width, uint32_t &height) {
        uint32_t depth = 1;
        calcMipDimensions(mip, width, height, depth);
    }
}
