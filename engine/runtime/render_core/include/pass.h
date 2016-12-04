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
 * @brief               Shader pass class.
 */

#pragma once

#include "core/hash_table.h"
#include "core/path.h"

#include "gpu/pipeline.h"

#include "render_core/defs.h"

#include <list>

class GPUCommandList;
class Shader;

/** Rendering pass. */
class Pass : Noncopyable {
public:
    /** Type of a list of shader variations. */
    using VariationList = std::list<ShaderKeywordSet>;

    /** Name of the basic pass type. */
    static const char *const kBasicType;

    Pass(Shader *parent, const std::string &type);
    ~Pass();

    /** @return             Parent shader. */
    Shader *parent() const { return m_parent; }
    /** @return             Type of the pass. */
    const std::string &type() const { return m_type.first; }

    bool loadStage(unsigned stage, const Path &path, const ShaderKeywordSet &keywords);

    void setDrawState(
        GPUCommandList *cmdList,
        const ShaderKeywordSet &variation = ShaderKeywordSet()) const;

    static void registerType(std::string type, VariationList variations);
private:
    /** Details of a pass type. */
    using Type = std::pair<const std::string, VariationList>;

    /** Structure holding a shader variation. */
    struct Variation {
        /** GPU pipeline. */
        GPUPipelinePtr pipeline;

        /** Set of programs for the pipeline (only valid before finalise()). */
        GPUProgramArray programs;
    };

    void finalise();

    static const Type &lookupType(const std::string &type);

    Shader *m_parent;               /**< Parent shader. */
    const Type &m_type;             /**< Type of the pass. */

    /**
     * Map of variations.
     *
     * The key to this map is a single string formed by concatenating all
     * keywords in the keyword set for the variation.
     */
    HashMap<std::string, Variation> m_variations;

    friend class Shader;
};
