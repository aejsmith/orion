/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Index data class.
 */

#include "gpu/index_data.h"

/**
 * Initialize the index data object.
 *
 * Initialize the index data object. The given buffer must be equal to the
 * number of indices multiplied by the size of an element of the given type.
 *
 * @param buffer        Buffer holding the index data.
 * @param type          Type of index elements.
 * @param count         Number of indices.
 */
GPUIndexData::GPUIndexData(GPUBuffer *buffer, Type type, size_t count) :
    m_buffer(buffer),
    m_type(type),
    m_count(count)
{
    check(buffer->type() == GPUBuffer::kIndexBuffer);
    check(buffer->size() == (elementSize() * count));
}
