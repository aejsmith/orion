/**
 * @file
 * @copyright           2014 Alex Smith
 * @brief               Rendering utility functions.
 */

#pragma once

#include "gpu/gpu.h"

/** Create a GPU buffer from an array of data.
 * @param type          Buffer type.
 * @param data          Array containing buffer data.
 * @param usage         Usage hint, defaults to GPUBuffer::kStaticDrawUsage.
 * @return              Pointer to created buffer. */
template <typename ElementType>
inline GPUBufferPtr buildGPUBuffer(
    GPUBuffer::Type type,
    const std::vector<ElementType> &data,
    GPUBuffer::Usage usage = GPUBuffer::kStaticDrawUsage)
{
    size_t size = data.size() * sizeof(ElementType);
    GPUBufferPtr buffer = g_gpu->createBuffer(type, usage, size);
    buffer->write(0, size, &data[0]);
    return buffer;
}

extern void makeQuad(GPUVertexDataPtr &vertices);
extern void makeSphere(unsigned rings, unsigned sectors, GPUVertexDataPtr &vertices, GPUIndexDataPtr &indices);
extern void makeCone(unsigned baseVertices, GPUVertexDataPtr &vertices, GPUIndexDataPtr &indices);
