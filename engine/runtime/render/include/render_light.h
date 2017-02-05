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
 * @brief               Renderer light class.
 */

#pragma once

#include "gpu/resource.h"

#include "render/render_view.h"

class RenderWorld;

struct Geometry;

/** Per-light uniform buffer structure. */
UNIFORM_STRUCT_BEGIN(LightUniforms)
    UNIFORM_STRUCT_MEMBER(glm::vec3, position);
    UNIFORM_STRUCT_MEMBER(float, intensity);
    UNIFORM_STRUCT_MEMBER(glm::vec3, direction);
    UNIFORM_STRUCT_MEMBER(float, cosCutoff);
    UNIFORM_STRUCT_MEMBER(glm::vec3, colour);
    UNIFORM_STRUCT_MEMBER(float, range);
    UNIFORM_STRUCT_MEMBER(glm::mat4, volumeTransform);
    UNIFORM_STRUCT_MEMBER(glm::mat4, shadowSpace);
    UNIFORM_STRUCT_MEMBER(float, shadowZNear);
    UNIFORM_STRUCT_MEMBER(float, attenuationConstant);
    UNIFORM_STRUCT_MEMBER(float, attenuationLinear);
    UNIFORM_STRUCT_MEMBER(float, attenuationExp);
UNIFORM_STRUCT_END;

/** Renderer representation of a light source. */
class RenderLight {
public:
    /** Type of a light. */
    enum Type {
        kAmbientLight,              /**< Ambient light. */
        kDirectionalLight,          /**< Directional light. */
        kPointLight,                /**< Point light. */
        kSpotLight,                 /**< Spot light. */
        kNumTypes,
    };

    /** Light flags. */
    enum : uint32_t {
        /** Whether the light casts a shadow. */
        kCastsShadows = (1 << 0),
    };

    /** Maximum number of shadow views. */
    static const size_t kMaxShadowViews = CubeFace::kNumFaces;

    explicit RenderLight(Type type);
    ~RenderLight();

    void setWorld(RenderWorld *world);

    void setPosition(const glm::vec3 &position);
    void setDirection(const glm::vec3 &direction);
    void setColour(const glm::vec3 &colour);
    void setIntensity(float intensity);
    void setCutoff(float cutoff);
    void setRange(float range);
    void setAttenuation(float constant, float linear, float exp);
    void setFlags(uint32_t flags);

    /** @return             Type of the light. */
    Type type() const { return m_type; }
    /** @return             Position of the light. */
    const glm::vec3 &position() const { return m_position; }
    /** @return             Direction of the light (directional/spot). */
    const glm::vec3 &direction() const { return m_direction; }
    /** @return             Colour that the light emits. */
    const glm::vec3 &colour() const { return m_colour; }
    /** @return             Diffuse intensity. */
    float intensity() const { return m_intensity; }
    /** @return             Angle of effect (spot). */
    float cutoff() const { return m_cutoff; }
    /** @return             Range of the light (point/spot). */
    float range() const { return m_range; }
    /** @return             Constant attenuation factor (point/spot). */
    float attenuationConstant() const { return m_attenuationConstant; }
    /** @return             Linear attenuation factor (point/spot). */
    float attenuationLinear() const { return m_attenuationLinear; }
    /** @return             Exponential attenuation factor (point/spot). */
    float attenuationExp() const { return m_attenuationExp; }
    /** @return             Flags for the light. */
    uint32_t flags() const { return m_flags; }
    /** @return             Whether the light casts shadows. */
    bool castsShadows() const { return (m_flags & kCastsShadows) != 0; }

    GPUResourceSet *getResources();

    Geometry volumeGeometry() const;

    /** @return             Number of shadow views for this light. */
    unsigned numShadowViews() const {
        return (m_type == kPointLight) ? CubeFace::kNumFaces : 1;
    }

    /** Get the shadow view at the specified index.
     * @param index         Index to get at.
     * @return              Pointer to shadow view. */
    RenderView *shadowView(unsigned index) {
        return &m_shadowViews[index];
    }

    bool cull(RenderView &view) const;

    #ifdef ORION_BUILD_DEBUG
    std::string name;               /**< Name of the light (used for debugging). */
    #endif
private:
    void updateVolumeTransform();
    void updateShadowViews();
    void updateWorld();
private:
    RenderWorld *m_world;           /**< World that this light belongs to. */

    Type m_type;                    /**< Type of the light. */

    glm::vec3 m_position;           /**< Position of the light. */
    glm::vec3 m_direction;          /**< Direction of the light (directional/spot). */
    glm::vec3 m_colour;             /**< Colour that the light emits. */
    float m_intensity;              /**< Diffuse intensity. */
    float m_cutoff;                 /**< Angle of effect (spot). */
    float m_range;                  /**< Range of the light (point/spot). */
    float m_attenuationConstant;    /**< Constant attenuation factor (point/spot). */
    float m_attenuationLinear;      /**< Linear attenuation factor (point/spot). */
    float m_attenuationExp;         /**< Exponential attenuation factor (point/spot). */
    uint32_t m_flags;               /**< Behaviour flags for the light. */

    /** Bounding box (for spot lights). */
    BoundingBox m_boundingBox;

    /** Deferred light volume transformation. */
    Transform m_volumeTransform;

    /** Uniform buffer containing lighting parameters. */
    UniformBuffer<LightUniforms> m_uniforms;

    /** Resource set containing per-light resource bindings. */
    GPUResourceSetPtr m_resources;

    /** Views for shadow map rendering. */
    RenderView m_shadowViews[kMaxShadowViews];
};
