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
 * @brief               Light component classes.
 */

#pragma once

#include "engine/component.h"

#include "render/scene_light.h"

/**
 * Base light component class.
 *
 * This component implements a light source in the world. This class cannot be
 * created directly, you must create one of the specific light type classes.
 */
class Light : public Component {
public:
    DECLARE_COMPONENT(Component::kLightType);
public:
    ~Light();

    void setColour(const glm::vec3 &colour);
    void setIntensity(float intensity);
    void setCastShadows(bool castShadows);

    /** @return             Colour that the light emits. */
    const glm::vec3 &colour() const { return m_sceneLight.colour(); }
    /** @return             Diffuse intensity. */
    float intensity() const { return m_sceneLight.intensity(); }
    /** @return             Whether the light casts shadows. */
    bool castShadows() const { return m_sceneLight.castShadows(); }
protected:
    Light(Entity *entity, SceneLight::Type type);

    void transformed() override;
    void activated() override;
    void deactivated() override;

    /**
     * Methods made public by derived classes that require them.
     */

    void setDirection(const glm::vec3 &direction);
    void setCutoff(float cutoff);
    void setRange(float range);
    void setAttenuation(float constant, float linear, float exp);

    glm::vec3 direction() const;

    /** @return             Angle of effect. */
    float cutoff() const { return m_sceneLight.cutoff(); }
    /** @return             Range of the light. */
    float range() const { return m_sceneLight.range(); }
    /** @return             Constant attenuation factor. */
    float attenuationConstant() const { return m_sceneLight.attenuationConstant(); }
    /** @return             Linear attenuation factor. */
    float attenuationLinear() const { return m_sceneLight.attenuationLinear(); }
    /** @return             Exponential attenuation factor. */
    float attenuationExp() const { return m_sceneLight.attenuationExp(); }
protected:
    /** Scene light implementing this light. */
    SceneLight m_sceneLight;
};

/**
 * Ambient light component.
 *
 * This component adds ambient lighting to the world. The ambient light is a
 * single colour value/intensity that is added on to the overall shading, to
 * simulate the effect of light scattered about the entire scene. The position
 * is ignored, the light affects the whole scene.
 */
class AmbientLight : public Light {
public:
    explicit AmbientLight(Entity *entity);
};

/**
 * Directional light component.
 *
 * This component adds a directional light source to the world. A directional
 * light is one that affects the whole world equally from a certain direction,
 * with no distance cutoff. The position of the light is irrelevant.
 *
 * The light direction is stored using the entity orientation: the default
 * direction is (0, 0, -1), and the orientation is applied to that to give the
 * light direction. Because of this, the actual light direction in the world is
 * affected by the parent entity's rotation.
 */
class DirectionalLight : public Light {
public:
    explicit DirectionalLight(Entity *entity);

    using Light::setDirection;
    using Light::direction;
};

/**
 * Point light component.
 *
 * This component adds a point light source to the world. A point light is a
 * light that radiates out from a point in the world. It has a limited range,
 * and attenuation across that range.
 */
class PointLight : public Light {
public:
    explicit PointLight(Entity *entity);

    using Light::setRange;
    using Light::setAttenuation;
    using Light::range;
    using Light::attenuationConstant;
    using Light::attenuationLinear;
    using Light::attenuationExp;
};

/**
 * Spot light component.
 *
 * This component adds a spot light source to the world. A spot light radiates
 * out in a cone in a certain direction from a point in the world. It has a
 * limited range, and attenuation across that range. See DirectionalLight for
 * details on how the light direction is stored.
 */
class SpotLight : public Light {
public:
    explicit SpotLight(Entity *entity);

    using Light::setDirection;
    using Light::setCutoff;
    using Light::setRange;
    using Light::setAttenuation;
    using Light::direction;
    using Light::cutoff;
    using Light::range;
    using Light::attenuationConstant;
    using Light::attenuationLinear;
    using Light::attenuationExp;
};

/** Set the colour of the light.
 * @param colour        New light colour. */
inline void Light::setColour(const glm::vec3 &colour) {
    m_sceneLight.setColour(colour);
}

/** Set the intensity of the light.
 * @param intensity     New light intensity. */
inline void Light::setIntensity(float intensity) {
    m_sceneLight.setIntensity(intensity);
}

/** Set whether the light casts shadows.
 * @param castShadows   Whether the light casts shadows. */
inline void Light::setCastShadows(bool castShadows) {
    m_sceneLight.setCastShadows(castShadows);
}

/** Set the cutoff angle.
 * @param cutoff        New cutoff angle. Must be <= 45 degrees. */
inline void Light::setCutoff(float cutoff) {
    m_sceneLight.setCutoff(cutoff);
}

/** Set the range of the light.
 * @param range         Range of the light. */
inline void Light::setRange(float range) {
    m_sceneLight.setRange(range);
}

/** Set the attenuation factors.
 * @param constant      Constant attenuation factor.
 * @param linear        Linear attenuation factor.
 * @param exp           Exponentional attenuation factor. */
inline void Light::setAttenuation(float constant, float linear, float exp) {
    m_sceneLight.setAttenuation(constant, linear, exp);
}
