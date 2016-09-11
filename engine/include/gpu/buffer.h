/*
 * Copyright (C) 2015-2016 Alex Smith
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
 * @brief               GPU buffer class.
 */

#pragma once

#include "gpu/defs.h"

#include <vector>

/**
 * Class for storing data on the GPU.
 *
 * This class encapsulates a buffer in GPU memory. There are multiple buffer
 * types, the type of the buffer must be declared at creation time. The
 * implementation of the class is API-specific, therefore instances must be
 * created with GPUManager::createBuffer().
 */
class GPUBuffer : public GPUObject {
public:
    /** Enum of possible buffer types. */
    enum Type {
        kVertexBuffer,              /**< Vertex buffer. */
        kIndexBuffer,               /**< Index buffer. */
        kUniformBuffer,             /**< Uniform buffer. */
    };

    /** Enum describing intended buffer usage. */
    enum Usage {
        /** Infrequently modified data. */
        kStaticUsage,

        /** Modified frequently, used multiple times. */
        kDynamicUsage,

        /** Modified once, used at most a few times within the current frame. */
        kTransientUsage,
    };

    /** Buffer mapping flags. */
    enum MapFlags : uint32_t {
        /**
         * Invalidate the entire buffer when mapping.
         *
         * This forces an invalidation of the entire buffer even if only
         * partially mapping it.
         */
        kMapInvalidateBuffer = (1 << 0),
    };

    /** Buffer mapping access flags. */
    enum Access {
        kWriteAccess,               /**< Map for writing. */
    };

    /**
     * Map the buffer.
     *
     * Map the buffer into the CPU address space. This function returns a
     * pointer through which the buffer contents can be accessed and modified.
     * When it is no longer needed it should be unmapped with unmap(). Note that
     * only one part of a buffer can be mapped at any one time.
     *
     * Mapping a range for write access will invalidate the contents of that
     * range, therefore users are expected to re-write the entire buffer
     * content.
     *
     * Mapping a subrange of the buffer may cause synchronization with the GPU
     * if any previous draw calls which access the data are still in progress.
     * To avoid this, the kMapInvalidateBuffer flag can be specified which will
     * invalidate the entire buffer content instead of just the subrange.
     *
     * @param offset        Offset to map from.
     * @param size          Size of the range to map.
     * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
     * @param access        Access mode.
     *
     * @return              Pointer to mapped buffer.
     */
    virtual void *map(size_t offset, size_t size, uint32_t flags, uint32_t access) = 0;

    /** Unmap the previous mapping created for the buffer with map(). */
    virtual void unmap() = 0;

    virtual void write(size_t offset, size_t size, const void *buf, uint32_t flags = 0);

    /** @return             Type of the buffer. */
    Type type() const { return m_type; }
    /** @return             Buffer usage hint. */
    Usage usage() const { return m_usage; }
    /** @return             Total buffer size. */
    size_t size() const { return m_size; }
protected:
    GPUBuffer(Type type, Usage usage, size_t size);

    /** Destroy the buffer. */
    ~GPUBuffer() {}

    Type m_type;                    /**< Type of the buffer */
    Usage m_usage;                  /**< Buffer usage hint. */
    size_t m_size;                  /**< Buffer size. */
};

/** Type of a pointer to a GPU buffer. */
using GPUBufferPtr = GPUObjectPtr<GPUBuffer>;

/** Type of a GPU buffer array. */
using GPUBufferArray = std::vector<GPUBufferPtr>;
