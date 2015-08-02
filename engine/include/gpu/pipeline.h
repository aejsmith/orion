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
 * @brief               Rendering pipeline object.
 */

#pragma once

#include "gpu/program.h"

/** Pipeline descriptor. */
struct GPUPipelineDesc {
    /** Array of GPU programs, indexed by stage. */
    GPUProgramArray programs;
};

/**
 * Rendering pipeline.
 *
 * This class groups together a set of GPU programs to use for each pipeline
 * stage. Once created, a pipeline is immutable. Creation is performed through
 * GPUManager::createPipeline().
 *
 * Modern APIs (DX12, Metal, Mantle) have the concept of pipeline state objects
 * that bundle up shaders along with some bits of state like blend mode, output
 * buffer format and vertex format, the goal being to avoid draw-time shader
 * recompilation for different states. Storing output buffer/vertex formats here
 * would be quite awkward to manage and would add unnecessary overhead for APIs
 * that do not need this. However, APIs that do can fairly easily cache created
 * state objects for different formats in their implementation of this class.
 */
class GPUPipeline : public GPUResource {
public:
    /** @return             Array of programs used by the pipeline. */
    const GPUProgramArray &programs() const { return m_programs; }
protected:
    explicit GPUPipeline(const GPUPipelineDesc &desc);
protected:
    GPUProgramArray m_programs;         /**< Array of programs for each stage. */

    /* For the default implementation of createPipeline(). */
    friend class GPUManager;
};

/** Type of a reference to GPUPipeline. */
typedef GPUResourcePtr<GPUPipeline> GPUPipelinePtr;
