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
 * @brief               Shader pipeline object.
 */

#pragma once

#include "gpu/program.h"
#include "gpu/resource.h"

/** Shader pipeline descriptor. */
struct GPUPipelineDesc {
    /** Array of GPU programs, indexed by stage. */
    GPUProgramArray programs;

    /** Array of resource set layouts, indexed by set number. */
    GPUResourceSetLayoutArray resourceLayout;
};

/**
 * Shader pipeline.
 *
 * This class groups together a set of GPU shader programs to use for each
 * pipeline stage, and a description of the resource set layouts that will be
 * used with the programs. Once created, a pipeline is immutable. Creation is
 * performed through GPUManager::createPipeline().
 *
 * Modern APIs (Vulkan, DX12, Metal) have the concept of pipeline state objects
 * that bundle up shaders along with a large amount of other state (e.g. depth/
 * stencil state, blending, vertex data layout, etc.), the goal being to avoid
 * draw-time state validation or shader recompilation for different combinations
 * of states. Storing all this state here in addition to the shaders would be
 * awkward to use for the higher level engine. Therefore, for these APIs we
 * instead create their monolithic pipeline objects dynamically based on the
 * other states set at the time of a draw call using a pipeline. These are
 * cached internally within the API-specific implementations of this class.
 * In most cases, after rendering for a short time we will have built up a cache
 * of all the pipelines we need. Furthermore, some of these APIs allow us to
 * cache the created pipelines to disk to further speed up creation.
 */
class GPUPipeline : public GPUObject {
public:
    /** @return             Array of resource set layouts. */
    const GPUResourceSetLayoutArray &resourceLayout() const { return m_resourceLayout; }
protected:
    explicit GPUPipeline(GPUPipelineDesc &&desc);
    ~GPUPipeline() {}

    GPUProgramArray m_programs;         /**< Array of programs for each stage. */

    /** Array of resource set layouts. */
    GPUResourceSetLayoutArray m_resourceLayout;
};

/** Type of a reference to GPUPipeline. */
using GPUPipelinePtr = GPUObjectPtr<GPUPipeline>;
