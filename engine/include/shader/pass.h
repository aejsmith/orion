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
 * @brief               Shader pass class.
 */

#pragma once

#include "core/path.h"

#include "gpu/pipeline.h"

#include <set>
#include <vector>

class SceneLight;
class Shader;

/** Rendering pass. */
class Pass : Noncopyable {
public:
    /** Pass types. */
    enum Type {
        /**
         * Always rendered, no lighting is applied. Also used for post-process
         * and internal shaders. Every pass of this type will be executed in
         * order once per entity.
         */
        kBasicPass,

        /**
         * Forward shading pass. Every pass of this type will be executed in
         * order for each light affecting the entity.
         */
        kForwardPass,

        /**
         * Deferred shading pass. Outputs material colours and properties to the
         * G-Buffer, which will be used to compute lighting. Only one pass of
         * this type should be specified.
         */
        kDeferredPass,

        /** Shadow caster pass. Used when rendering shadow maps. */
        kShadowCasterPass,

        /** Number of Pass types. */
        kNumTypes,
    };

    /** Set of shader variation keywords. */
    using KeywordSet = std::set<std::string>;

    Pass(Shader *parent, Type type);
    ~Pass();

    /** @return             Parent shader. */
    Shader *parent() const { return m_parent; }
    /** @return             Type of the pass. */
    Type type() const { return m_type; }

    bool loadStage(unsigned stage, const Path &path, const KeywordSet &keywords);

    void setDrawState(SceneLight *light) const;
private:
    /** Structure holding a shader variation. */
    struct Variation {
        /** Descriptor for the pipeline. */
        GPUPipelineDesc desc;

        /** Pipeline created for the stage. */
        GPUPipelinePtr pipeline;
    };

    void finalize();

    Shader *m_parent;               /**< Parent shader. */
    Type m_type;                    /**< Type of the pass. */

    /**
     * Array of shader variations. See setDrawState() for how the array is
     * indexed.
     */
    std::vector<Variation> m_variations;

    friend class Shader;
};
