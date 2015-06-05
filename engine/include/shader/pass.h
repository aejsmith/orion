/**
 * @file
 * @copyright           2015 Alex Smith
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
    typedef std::set<std::string> KeywordSet;
public:
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
private:
    void finalize();
private:
    Shader *m_parent;               /**< Parent shader. */
    Type m_type;                    /**< Type of the pass. */

    /**
     * Array of shader variations. See setDrawState() for how the array is
     * indexed.
     */
    std::vector<Variation> m_variations;

    friend class Shader;
};
