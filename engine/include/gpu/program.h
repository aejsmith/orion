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
 * @brief               GPU program class.
 */

#pragma once

#include "gpu/defs.h"

#include <array>
#include <vector>

/** Descriptor for a GPU program. */
struct GPUProgramDesc {
    unsigned stage;                     /**< Type of the program. */
    std::vector<uint32_t> spirv;        /**< SPIR-V binary. */
    std::string name;                   /**< Name of the program. */
};

/** GPU program class. */
class GPUProgram : public GPUObject {
public:
    /** @return             Stage that the program is for. */
    unsigned stage() const { return m_stage; }
protected:
    /** Initialize the program.
     * @param stage         Stage that the program is for. */
    explicit GPUProgram(unsigned stage) : m_stage(stage) {}

    ~GPUProgram() {}
private:
    unsigned m_stage;                   /**< Type of the program. */
};

/** Type of a GPU program pointer. */
using GPUProgramPtr = GPUObjectPtr<GPUProgram>;

/** Type of an array of GPU program references, indexed by stage. */
using GPUProgramArray = std::array<GPUProgramPtr, ShaderStage::kNumStages>;
