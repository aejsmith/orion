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

/**
 * Details of a pass type.
 *
 * Use the DEFINE_PASS_TYPE macro at global scope to define a pass type which
 * will be registered at initialisation.
 */
class PassType {
public:
    /** Type of a list of shader variations. */
    using VariationList = std::list<ShaderKeywordSet>;

    PassType(std::string name, VariationList variations);
    ~PassType();

    std::string name;
    VariationList variations;

    static const PassType &lookup(const std::string &name);
};

/* Bypass preprocessor idiocy. */
#define _PASS_TYPE_INSTANCE_NAME(id) g_passType_ ## id
#define PASS_TYPE_INSTANCE_NAME(id) _PASS_TYPE_INSTANCE_NAME(id)

/** Define a pass type (use at global scope).
 * @see                 PassType::PassType(). */
#define DEFINE_PASS_TYPE(...) \
    static PassType PASS_TYPE_INSTANCE_NAME(__LINE__)(__VA_ARGS__)

/** Rendering pass. */
class Pass : Noncopyable {
public:
    /** Name of the basic pass type. */
    static const std::string kBasicType;

    Pass(Shader *parent, const std::string &type);
    ~Pass();

    /** @return             Parent shader. */
    Shader *parent() const { return m_parent; }
    /** @return             Type of the pass. */
    const std::string &type() const { return m_type.name; }

    bool loadStage(unsigned stage, const Path &path, const ShaderKeywordSet &keywords);

    void setDrawState(
        GPUCommandList *cmdList,
        const ShaderKeywordSet &variation = ShaderKeywordSet()) const;
private:
    /** Structure holding a shader variation. */
    struct Variation {
        /** GPU pipeline. */
        GPUPipelinePtr pipeline;

        /** Set of programs for the pipeline (only valid before finalise()). */
        GPUProgramArray programs;
    };

    void finalise();

    Shader *m_parent;               /**< Parent shader. */
    const PassType &m_type;         /**< Type of the pass. */

    /**
     * Map of variations.
     *
     * The key to this map is a single string formed by concatenating all
     * keywords in the keyword set for the variation.
     */
    HashMap<std::string, Variation> m_variations;

    friend class Shader;
};
