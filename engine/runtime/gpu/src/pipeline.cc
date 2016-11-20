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

#include "gpu/pipeline.h"

/** Initialize the pipeline.
 * @param desc          Parameters for the pipeline. */
GPUPipeline::GPUPipeline(GPUPipelineDesc &&desc) :
    m_programs(std::move(desc.programs)),
    m_resourceLayout(std::move(desc.resourceLayout))
{
    checkMsg(
        m_programs[ShaderStage::kVertex] && m_programs[ShaderStage::kFragment],
        "A pipeline requires at least a vertex and a fragment program");
}
