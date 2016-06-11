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
 * @brief               Index data class.
 */

#pragma once

#include "gpu/buffer.h"

/**
 * Class which collects index data.
 *
 * This class holds a buffer containing index data and information about the
 * format of that buffer. It is used to provide indices into vertex data to be
 * used for rendering. Since this class may have an API-specific implementation,
 * instances must be created with GPUManager::createIndexData().
 */
class GPUIndexData : public GPUResource {
public:
    /** Type of index elements. */
    enum Type {
        kUnsignedByteType,          /**< Unsigned 8-bit. */
        kUnsignedShortType,         /**< Unsigned 16-bit. */
        kUnsignedIntType,           /**< Unsigned 32-bit. */
    };

    /** @return             Buffer containing index data. */
    GPUBuffer *buffer() const { return m_buffer; }
    /** @return             Type of index elements. */
    Type type() const { return m_type; }
    /** @return             Number of indices. */
    size_t count() const { return m_count; }
    /** @return             Offset of the indices in the buffer. */
    size_t offset() const { return m_offset; }
    /** @return             Size of a single index element. */
    size_t elementSize() const { return elementSize(m_type); }

    /** Get the size of a buffer element of a certain type.
     * @param type          Type of buffer element.
     * @return              Size of element. */
    static size_t elementSize(Type type) {
        switch (type) {
        case kUnsignedByteType:
            return 1;
        case kUnsignedShortType:
            return 2;
        case kUnsignedIntType:
            return 4;
        default:
            return 0;
        }
    }
protected:
    GPUIndexData(GPUBuffer *buffer, Type type, size_t count, size_t offset);
    ~GPUIndexData() {}

    GPUBufferPtr m_buffer;          /**< Buffer containing index data. */
    Type m_type;                    /**< Type of index elements. */
    size_t m_count;                 /**< Number of indices. */
    size_t m_offset;                /**< Offset of the indices in the buffer. */

    /* For the default implementation of createIndexData(). */
    friend class GPUManager;
};

/** Type of a reference to GPUIndexData */
typedef GPUResourcePtr<GPUIndexData> GPUIndexDataPtr;
