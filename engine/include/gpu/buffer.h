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
        /** Stream: modified once and used at most a few times. */
        kStreamDrawUsage,           /**< Used for drawing operations. */
        kStreamReadUsage,           /**< Used to read from. */
        kStreamCopyUsage,           /**< Used for reading and drawing. */

        /** Static: modified once and used many times. */
        kStaticDrawUsage,           /**< Used for drawing operations. */
        kStaticReadUsage,           /**< Used to read from. */
        kStaticCopyUsage,           /**< Used for reading and drawing. */

        /** Dynamic: modified repeatedly and used many times. */
        kDynamicDrawUsage,          /**< Used for drawing operations. */
        kDynamicReadUsage,          /**< Used to read from. */
        kDynamicCopyUsage,          /**< Used for reading and drawing. */
    };

    /** Buffer mapping flags. */
    enum MapFlags : uint32_t {
        /**
         * Invalidate the range contents when mapping.
         *
         * This flag should be used when the entire contents of the mapped range
         * is to be overwritten. If mapping the whole buffer, the whole buffer
         * will be invalidated, allowing the driver to reallocate the buffer and
         * avoid synchronization with any previous draw calls still using the
         * buffer. Note that this can be useful even on partial ranges when
         * mapping write-only, as this allows the driver to return temporary
         * memory to avoid synchronization.
         */
        kMapInvalidate = (1 << 0),

        /**
         * Invalidate the entire buffer when mapping.
         *
         * This forces an invalidation of the entire buffer even if only
         * partially mapping it.
         */
        kMapInvalidateBuffer = (1 << 1),
    };

    /** Buffer mapping access flags. */
    enum AccessFlags : uint32_t {
        /** Map for reading. */
        kReadAccess = (1 << 0),
        /** Map for writing. */
        kWriteAccess = (1 << 1),
    };

    void write(size_t offset, size_t size, const void *buf);
    void *map(size_t offset, size_t size, uint32_t flags, uint32_t access);
    void unmap();

    /** @return             Type of the buffer. */
    Type type() const { return m_type; }
    /** @return             Buffer usage hint. */
    Usage usage() const { return m_usage; }
    /** @return             Total buffer size. */
    size_t size() const { return m_size; }
protected:
    GPUBuffer(Type type, Usage usage, size_t size);
    ~GPUBuffer();

    /** API-specific implementation of write(). */
    virtual void writeImpl(size_t offset, size_t size, const void *buf) = 0;

    /** API-specific implementation of map(). */
    virtual void *mapImpl(size_t offset, size_t size, uint32_t flags, uint32_t access) = 0;

    /** API-specific implementation of unmap(). */
    virtual void unmapImpl() = 0;

    Type m_type;                    /**< Type of the buffer */
    Usage m_usage;                  /**< Buffer usage hint. */
    size_t m_size;                  /**< Buffer size. */
    bool m_mapped;                  /**< Whether the buffer is currently mapped. */
};

/** Type of a pointer to a GPU buffer. */
using GPUBufferPtr = GPUObjectPtr<GPUBuffer>;

/** Type of a GPU buffer array. */
using GPUBufferArray = std::vector<GPUBufferPtr>;

/**
 * Scoped buffer mapper class.
 *
 * This class is an RAII class which will map a GPUBuffer, and unmap it once it
 * goes out of scope. The object behaves as a pointer of the specified type,
 * through which the buffer contents can be accessed.
 *
 * @tparam T        Type of the data contained in the buffer.
 */
template <typename T>
class GPUBufferMapper : Noncopyable {
public:
    /** Map the entire buffer.
     * @see                 GPUBuffer::map(). */
    GPUBufferMapper(GPUBuffer *buffer, uint32_t flags, uint32_t access) :
        m_buffer(buffer)
    {
        m_mapping = reinterpret_cast<T *>(m_buffer->map(0, buffer->size(), flags, access));
    }

    /** Map a range of the buffer.
     * @see                 GPUBuffer::map(). */
    GPUBufferMapper(GPUBuffer *buffer, size_t offset, size_t size, uint32_t flags, uint32_t access) :
        m_buffer(buffer)
    {
        m_mapping = reinterpret_cast<T *>(m_buffer->map(offset, size, flags, access));
    }

    /** Unmap the buffer. */
    ~GPUBufferMapper() {
        m_buffer->unmap();
    }

    /** Get the mapping.
     * @return              Pointer to mapping. Valid while this object is
     *                      still in scope. */
    T *get() const { return m_mapping; }

    T &operator *() const { return *m_mapping; }
    T *operator ->() const { return m_mapping; }
    T &operator [](size_t n) const { return m_mapping[n]; }
private:
    /** Buffer being mapped.
     * @note                Hold just a raw pointer as the user should hold
     *                      a reference to the buffer. */
    GPUBuffer *m_buffer;

    /** Pointer to mapping. */
    T *m_mapping;
};
