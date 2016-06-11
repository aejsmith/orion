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
 * @brief               GPU program class.
 */

#pragma once

#include "gpu/defs.h"

#include <array>
#include <list>

/** GPU program class. */
class GPUProgram : public GPUResource {
public:
    /** Structure describing a resource. */
    struct Resource {
        std::string name;           /**< Name of the resource. */
        unsigned index;             /**< Index of the resource for use with bind functions. */
    };

    /** Type of a resource list, given as a pair of name and index.
     * @note                List rather than a vector to allow for sparse
     *                      indices. */
    typedef std::list<Resource> ResourceList;

    /** @return             Stage that the program is for. */
    unsigned stage() const { return m_stage; }

    /** Query active uniform blocks in the program.
     * @param list          Resource list to fill in. */
    virtual void queryUniformBlocks(ResourceList &list) = 0;

    /** Query active texture samplers in the program.
     * @param list          Resource list to fill in. */
    virtual void querySamplers(ResourceList &list) = 0;

    /**
     * Bind a uniform block in the program.
     *
     * Specifies that the uniform block at the specified index (as returned
     * from queryUniformBlocks()) should refer to the uniform buffer which is
     * bound in the specified slot at the time of a draw call involving the
     * program.
     *
     * @param index         Index of uniform block.
     * @param slot          Uniform buffer slot.
     */
    virtual void bindUniformBlock(unsigned index, unsigned slot) = 0;

    /**
     * Bind a texture sampler in the program.
     *
     * Specifies that the texture sampler at the specified index (as returned
     * from querySamplers()) should refer to the texture which is bound in the
     * specified slot at the time of a draw call involving the program.
     *
     * @param index         Index of sampler.
     * @param slot          Texture slot.
     */
    virtual void bindSampler(unsigned index, unsigned slot) = 0;
protected:
    /** Initialize the program.
     * @param stage         Stage that the program is for. */
    explicit GPUProgram(unsigned stage) : m_stage(stage) {}

    ~GPUProgram() {}
private:
    unsigned m_stage;                   /**< Type of the program. */
};

/** Type of a GPU program pointer. */
typedef GPUResourcePtr<GPUProgram> GPUProgramPtr;

/** Type of an array of GPU programs, indexed by stage. */
typedef std::array<GPUProgramPtr, ShaderStage::kNumStages> GPUProgramArray;
