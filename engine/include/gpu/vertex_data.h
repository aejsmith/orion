/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Vertex data class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/vertex_format.h"

#include <vector>

/**
 * Class which collects vertex data information.
 *
 * This class collects one or more vertex buffers and a vertex format describing
 * the vertex attributes which are contained in the buffers. Once created, a
 * vertex data object is immutable. The vertex buffer contents can be changed,
 * but to change the vertex count or the buffers in use, a new vertex data
 * object must be created. Creation is performed through
 * GPUInterface::createVertexData().
 */
class GPUVertexData : public GPUResource {
public:
    /** @return             Total number of vertices. */
    size_t count() const { return m_count; }
    /** @return             Pointer to vertex format descriptor. */
    GPUVertexFormat *format() const { return m_format; }
    /** @return             GPU buffer array. */
    const GPUBufferArray &buffers() const { return m_buffers; }
protected:
    GPUVertexData(size_t count, GPUVertexFormat *format, GPUBufferArray &buffers);
protected:
    size_t m_count;                 /**< Vertex count. */
    GPUVertexFormatPtr m_format;    /**< Vertex format. */
    GPUBufferArray m_buffers;       /**< Vector of vertex buffers. */

    /* For the default implementation of createVertexData(). */
    friend class GPUInterface;
};

/** Type of a reference to GPUVertexData. */
typedef GPUResourcePtr<GPUVertexData> GPUVertexDataPtr;
