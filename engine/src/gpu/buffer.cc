/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               GPU buffer class.
 */

#include "gpu/buffer.h"

/** Construct the GPU buffer.
 * @param type          Type of the buffer.
 * @param usage         Usage hint.
 * @param size          Buffer size. */
GPUBuffer::GPUBuffer(Type type, Usage usage, size_t size) :
    m_type(type),
    m_usage(usage),
    m_size(size),
    m_mapped(false)
{}

/** Destroy the buffer. */
GPUBuffer::~GPUBuffer() {
    checkMsg(!m_mapped, "Destroying buffer which is still mapped");
}

/**
 * Write data to the buffer.
 *
 * Replaces some or all of the current buffer content with new data. The area
 * to write must lie within the bounds of the buffer, i.e. (offset + size) must
 * be less than or equal to the buffer size.
 *
 * @param offset        Offset to write at.
 * @param size          Size of the data to write.
 * @param buf           Buffer containing data to write.
 */
void GPUBuffer::write(size_t offset, size_t size, const void *buf) {
    checkMsg(!m_mapped, "Call to write() while buffer mapped");
    checkMsg(
        (offset + size) <= m_size,
        "Write outside buffer bounds (total: %zu, offset: %zu, size: %zu)", m_size, offset, size);

    writeImpl(offset, size, buf);
}

/**
 * Map the buffer.
 *
 * Map the buffer into the CPU address space. This function returns a pointer
 * through which the buffer contents can be accessed and modified. When it is
 * no longer needed it should be unmapped with unmap(). Note that only one part
 * of a buffer can be mapped at any one time.
 *
 * Mapping the buffer may cause synchronization with the GPU if any previous
 * draw calls which access the data are still in progress. If possible, you
 * should make use of invalidation to avoid this overhead.
 *
 * @param offset        Offset to map from.
 * @param size          Size of the range to map.
 * @param flags         Bitmask of mapping behaviour flags (see MapFlags).
 * @param access        Bitmask of access flags.
 *
 * @return              Pointer to mapped buffer.
 */
void *GPUBuffer::map(size_t offset, size_t size, uint32_t flags, uint32_t access) {
    checkMsg(!m_mapped, "Cannot create multiple buffer mappings");
    checkMsg(
        (offset + size) <= m_size,
        "Map outside buffer bounds (total: %zu, offset: %zu, size: %zu)", m_size, offset, size);
    check(!((flags & kMapInvalidate) && (flags & kMapInvalidateBuffer)));

    /* Convert invalidate range to invalidate buffer if the whole buffer is
     * specified. */
    if (flags & kMapInvalidate && offset == 0 && size == m_size)
        flags = (flags & ~kMapInvalidate) | kMapInvalidateBuffer;

    void *ret = mapImpl(offset, size, flags, access);
    m_mapped = true;
    return ret;
}

/** Unmap the previous mapping created for the buffer with map(). */
void GPUBuffer::unmap() {
    checkMsg(m_mapped, "Unmapping buffer which is not currently mapped");

    unmapImpl();
    m_mapped = false;
}
