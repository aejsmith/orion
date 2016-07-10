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
 * @brief               Vertex data class.
 */

#pragma once

#include "gpu/buffer.h"
#include "gpu/state.h"

#include <vector>

/**
 * Class which collects vertex data information.
 *
 * This class collects one or more vertex buffers and a vertex input state
 * describing the vertex attributes which are contained in the buffers. Once
 * created, a vertex data object is immutable. The vertex buffer contents can
 * be changed, but to change the vertex count or the buffers in use, a new
 * vertex data object must be created. Creation is performed through
 * GPUManager::createVertexData().
 */
class GPUVertexData : public GPUObject {
public:
    /** @return             Total number of vertices. */
    size_t count() const { return m_count; }
    /** @return             Pointer to vertex input state. */
    GPUVertexInputState *inputState() const { return m_inputState; }
    /** @return             GPU buffer array. */
    const GPUBufferArray &buffers() const { return m_buffers; }
protected:
    GPUVertexData(size_t count, GPUVertexInputState *inputState, GPUBufferArray &&buffers);
    ~GPUVertexData() {}

    size_t m_count;                         /**< Vertex count. */
    GPUVertexInputStatePtr m_inputState;    /**< Vertex input state. */
    GPUBufferArray m_buffers;               /**< Vector of vertex buffers. */

    /* For the default implementation of createVertexData(). */
    friend class GPUManager;
};

/** Type of a reference to GPUVertexData. */
using GPUVertexDataPtr = GPUObjectPtr<GPUVertexData>;
